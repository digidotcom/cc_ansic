import sys
import magic
import re
from os import listdir
from os.path import join, isdir, samefile
from collections import defaultdict
from argparse import ArgumentParser, REMAINDER

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
	ordered = sorted(amount)
	for text in ordered:
		show = '"{0}": {1}' if has_space.search(text) else '{0}: {1}'
		result.append(show.format(text, amount[text]))

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
			print '{0}: {1}'.format(num, line.rstrip('\r\n'))
			print show_counts(amount)
			
		add_counts(grand, total)
		print
		
def scan_dir(dir, exclude):
	subdirs = []
	files = []
	for partial in sorted(listdir(dir)):
		if partial == '.git' or partial == pattern_file:
			continue

		path = join(dir, partial)
		if exclude(path):
			continue
			
		if isdir(path):
			subdirs.append(path)
		else:
			files.append(path)

	for file in files:
		scan_file(file)
	for dir in subdirs:
		scan_dir(dir, exclude)

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
				leave('error parsing line number {0} of pattern file'.format(num))
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
	
parser = ArgumentParser(description='Locates rebranding terms.')
parser.add_argument('-x', '--exclude', action='append', metavar='PATH', help='directory or file to exclude (may be used multiple times)')
parser.add_argument('path', nargs='?', default='.', help='directory or file to scan')
parser.add_argument('remaining', nargs=REMAINDER)
args = parser.parse_args()

if args.remaining:
	print 'too many arguments'
	parser.print_help()
	sys.exit(1)

def exclude_path(paths):
	def exclude(check):
		for path in paths:
			if samefile(check, path):
				return True
		return False
		
	def never(path):
		return False

	return exclude if paths else never

exclude = exclude_path(args.exclude)
		
if exclude(args.path):
	print 'nothing to do'
	sys.exit(0)
	
print 'Search Terms'
print '------------'
read_patterns()
print

if isdir(args.path):
	scan_dir(args.path, exclude)
else:
	scan_file(args.path)

if skipped:
	print 'Skipped'
	print '-------'
	for file, content in skipped.iteritems():
		print '{0}: {1}'.format(file, content)
	print

if len(grand) == 0:
	print 'No terms matched'
else:
	print 'Totals'
	print '------'
	print show_counts(grand)

sys.exit(0)
