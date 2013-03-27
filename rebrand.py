import sys
import os.path
import magic
import re
from collections import defaultdict

def leave(text):
	sys.stderr.write(text + '\n')
	sys.exit(1)

def add_counts(current, more):
	for text, count in more.iteritems():
		if count != 0:
			current[text] += count
		
has_space = re.compile('\s')
def show_counts(amount):
	result = []
	for text, count in amount.iteritems():
		show = '"{}": {}' if has_space.search(text) else '{}: {}'
		result.append(show.format(text, count))

	return ', '.join(result)
	
def scan_str(string):
	def search_str(string, search, exclude):
		nocase = string.lower()
		count = nocase.count(search)
		for exclusion in exclude:
			count -= nocase.count(exclusion)
		return {search: max(count, 0)}

	total = defaultdict(int)
	
	for search, exclude in pattern.iteritems():
		add_counts(total, search_str(string, search, exclude))
		
	return total
	
grand = defaultdict(int)
skipped = {}
is_text = re.compile(' text')
def scan_file(file):
	result = []
	total = defaultdict(int)
	
	instance = scan_str(file)
	if instance:
		add_counts(total, instance)

	content = magic.from_file(file)
	if not is_text.search(content):
		skipped[file] = content
	else:
		num = 1
		for line in open(file):
			amount = scan_str(line)
			if amount:
				result.append([num, line, amount])
				add_counts(total, amount)
			num += 1

	if total:
		print file
		print '-' * len(file)
		print show_counts(total)
		
		if instance:
			print 'in pathname'
			print show_counts(instance)
			
		for num, line, amount in result:
			print '{}: {}'.format(num, line.rstrip('\r\n'))
			print show_counts(amount)
			
		add_counts(grand, total)
		print
		
def scan_dir(dir):
	subdirs = []
	files = []
	for partial in sorted(os.listdir(dir)):
		if partial == '.git' or partial == pattern_file:
			continue

		path = os.path.join(dir, partial)
		if os.path.isdir(path):
			subdirs.append(path)
		else:
			files.append(path)

	for file in files:
		scan_file(file)
	for dir in subdirs:
		scan_dir(dir)

pattern_file = 'rebrand.txt'
pattern = {}
def read_patterns():
	def parse_patterns(line, num):
		def parse_pattern(line, num):
			assert(len(line[0].strip()) != 0)
			if line[0] == '"':
				start = 1
				end_char = '"'
			elif line[0] == "'":
				start = 1
				end_char = "'"
			else:
				start = 0
				end_char = None
			try:
				text, remaining = line[start:].split(end_char, 1)
				remaining = remaining.strip()
			except ValueError:
				text = line
				remaining = ''
			if len(text) == 0:
				leave('error parsing line number {} of pattern file'.format(num))
			return text, remaining
				
		search, remaining = parse_pattern(line, num)
		exclude = []
		while len(remaining) != 0:
			if remaining[0] == '#':
				break
			text, remaining = parse_pattern(remaining, num)
			exclude.append(text.lower())
		return search.lower(), exclude

	num = 1
	for line in open(pattern_file):
		line = line.strip()
		if len(line) == 0:
			continue
		if line[0] == '#':
			continue
		search, exclude = parse_patterns(line, num)
		print search, exclude
		
		pattern[search] = exclude

print 'Search Terms'
print '------------'
read_patterns()
print

if len(sys.argv) == 1:
	path = '.'
else:
	path = sys.argv[1]
if os.path.isdir(path):
	scan_dir(path)
else:
	scan_file(path)

if skipped:
	print 'Skipped'
	print '-------'
	for file, content in skipped.iteritems():
		print '{}: {}'.format(file, content)
	print

print 'Totals'
print '------'
print show_counts(grand)

sys.exit(0)
