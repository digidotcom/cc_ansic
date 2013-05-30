import shutil
from tempfile import mkstemp
import os
from glob import glob
import time

root_dir = os.getcwd() + "\\..\\..\\.."					# Where the GIT repository is

cw_workspace = "C:\\Etherios4Kinetis\\Projects\\MQX\\CW10 Projects" 	# Where the CW template project is and where the created examples will be
iar_workspace = "C:\\Etherios4Kinetis\\Projects\\MQX\\IAR Projects"  	# Where the IAR template project is and where the created examples will be
template_name = "etherios_app" 							# The template application name (and folder)
template_dir_cw = cw_workspace + "\\" + template_name		
template_dir_iar = iar_workspace + "\\" + template_name

# Replace the whole line that contains "pattern" by "subst" in "file_path"
def replace(file_path, pattern, subst):
    #Create temp file
    fh, abs_path = mkstemp()
    new_file = open(abs_path,'w')
    old_file = open(file_path)
    for line in old_file:
        new_file.write(line.replace(pattern, subst))
    #close temp file
    new_file.close()
    os.close(fh)
    old_file.close()
    #Remove original file
    os.remove(file_path)
    #Move new file
    shutil.move(abs_path, file_path)

# Replaces the template project's name for the new name in Eclipse project's .project file, that's enough in CW
def change_cw_project_name(sample_dir, new_name):
	#sample_dir is the sample's root
	template_name = "etherios_application"
	replace(sample_dir + "\\.project", template_name, new_name) #these ".file" are not handled by he "for" loop
	replace(sample_dir + "\\.cproject", template_name, new_name) #these ".file" are not handled by he "for" loop

	for file in glob(sample_dir + "\\*.*"):
		replace(file, template_name, new_name)
		if template_name in file:
			new_filename = file.replace(template_name, new_name)
			os.rename(file, new_filename)
	
# Renames template_project.ewp by the new_name.ewp and replaces the reference in the IAR workspace (.eww)
def change_iar_project_name(sample_dir, new_name):
	#sample_dir is the sample's root
	template_name = "etherios_app"
	for file in glob(sample_dir + "\\*.*"):
		replace(file, template_name, new_name)
		if template_name in file:
			new_filename = file.replace(template_name, new_name)
			os.rename(file, new_filename)
	#replace(sample_dir + "\\etherios_app.eww", template_name, "<path>$WS_DIR$\\" + new_name + ".ewp</path>")
	#os.rename(sample_dir + "\\etherios_app.ewp", sample_dir + "\\" + new_name + ".ewp")
	#os.rename(sample_dir + "\\etherios_app.eww", sample_dir + "\\" + new_name + ".eww")
	#os.rename(sample_dir + "\\etherios_app.ewd", sample_dir + "\\" + new_name + ".ewd")
	#os.rename(sample_dir + "\\etherios_app.dep", sample_dir + "\\" + new_name + ".dep")

# This hard-coded function adds the Etherios TWR Demo files to IAR project, it's not necessary in CW because it adds all files in a folder
def add_ewdemo_files_to_iar_project(iar_ewp_file):
	files_to_add = ("Accel_Task.c", "adc16.c", "adc16.h", "ADC_Task.c", "cpu_usage.c", "DemoIO.c", "DemoIO.h", "Game_Task.c", "tower_demo.c", "tower_demo.h", "tsi.c", "tsi.h")
	group_name = "Demo Files"
	group_tag = "  <group>\n"
	group_tag_close = "  </group>\n"
	file_tag = "\n\n<file>\n"
	file_tag_close = "\n\n</file>\n"
	name_tag = "\n\n<name>"
	name_tag_close = "</name>\n"
	#Create temp file
	fh, abs_path = mkstemp()
	new_file = open(abs_path,'w')
	old_file = open(iar_ewp_file)
	for line in old_file:
		new_file.write(line)
		if "<name>Source</name>" in line:
			new_file.write(group_tag + name_tag + group_name + name_tag_close)
			for file in files_to_add:
				full_file_path = "$PROJ_DIR$\\Sources\\" + file
				new_file.write(file_tag + name_tag + full_file_path + name_tag_close + file_tag_close)
			new_file.write(group_tag_close)
	#close temp file
	new_file.close()
	os.close(fh)
	old_file.close()
	#Remove original file
	os.remove(iar_ewp_file)
	#Move new file
	shutil.move(abs_path, iar_ewp_file)

# This hard-coded function adds the RCI files to BASIC RCI project. Not necessary in CW
def add_basic_rci_files_to_iar_project(iar_ewp_file):
	files_to_add = ("gps_stats.c", "system.c", "remote_config.h", "remote_config_cb.h", "remote_config_cb.c")
	file_tag = "\n\n<file>\n"
	file_tag_close = "\n\n</file>\n"
	name_tag = "\n\n<name>"
	name_tag_close = "</name>\n"
	#Create temp file
	fh, abs_path = mkstemp()
	new_file = open(abs_path,'w')
	old_file = open(iar_ewp_file)
	for line in old_file:
		new_file.write(line)
		if "<name>Source</name>" in line:
			for file in files_to_add:
				full_file_path = "$PROJ_DIR$\\Sources\\" + file
				new_file.write(file_tag + name_tag + full_file_path + name_tag_close + file_tag_close)
	#close temp file
	new_file.close()
	os.close(fh)
	old_file.close()
	#Remove original file
	os.remove(iar_ewp_file)
	#Move new file
	shutil.move(abs_path, iar_ewp_file)
	
# This functions copies the template application, changes its name by "example_name" and replaces demo-specific files from GIT repository
def replicate_example(example_name, cw_workspace, iar_workspace):
	dest_sample_dir_cw = cw_workspace + "\\" + example_name
	dest_sample_dir_iar = iar_workspace + "\\" + example_name
	# For CodeWarrior
	if os.path.exists(dest_sample_dir_cw):
		shutil.rmtree(dest_sample_dir_cw)
	shutil.copytree(template_dir_cw, dest_sample_dir_cw)
	# For IAR
	if os.path.exists(dest_sample_dir_iar):
		shutil.rmtree(dest_sample_dir_iar)
	shutil.copytree(template_dir_iar, dest_sample_dir_iar)

	for file in glob(root_dir + "\\kits\\kinetis\\mqx\\samples\\" + example_name  + "\\*.c"):
		shutil.copy(file, dest_sample_dir_cw + "\\Sources")
		shutil.copy(file, dest_sample_dir_iar + "\\Sources")

	for file in glob(root_dir + "\\kits\\kinetis\\mqx\\samples\\" + example_name  + "\\*.h"):
		shutil.copy(file, dest_sample_dir_cw + "\\Sources")
		shutil.copy(file, dest_sample_dir_iar + "\\Sources")

	for file in glob(root_dir + "\\kits\\kinetis\\mqx\\samples\\" + example_name  + "\\*.txt"):
		shutil.copy(file, dest_sample_dir_cw)
		shutil.copy(file, dest_sample_dir_iar)

	change_cw_project_name(dest_sample_dir_cw, example_name)
	change_iar_project_name(dest_sample_dir_iar, example_name)

def copy_rci_files(example_name, cw_workspace, iar_workspace):
	dest_sample_dir_cw = cw_workspace + "\\" + example_name
	dest_sample_dir_iar = iar_workspace + "\\" + example_name

	#for file in glob(root_dir + "\\public\\run\\samples\\simple_remote_config\\*.c"):
	for file in glob(root_dir + "\\public\\run\\samples\\simple_remote_config\\*.c"):
		if ("application.c" not in file) and ("status.c" not in file):
			shutil.copy(file, dest_sample_dir_cw + "\\Sources")
			shutil.copy(file, dest_sample_dir_iar + "\\Sources")
	for file in glob(root_dir + "\\public\\run\\samples\\simple_remote_config\\*.h"):
		if ("connector_config.h" not in file):
			shutil.copy(file, dest_sample_dir_cw + "\\Sources")
			shutil.copy(file, dest_sample_dir_iar + "\\Sources")
	for file in glob(root_dir + "\\public\\run\\samples\\simple_remote_config\\*.rci"):
		shutil.copy(file, dest_sample_dir_cw + "\\Sources")
		shutil.copy(file, dest_sample_dir_iar + "\\Sources")


# main application 
def main():
	replicate_example("send_data", cw_workspace, iar_workspace)
	replicate_example("device_request", cw_workspace, iar_workspace)
	replicate_example("ewdemo", cw_workspace, iar_workspace)
	replicate_example("data_points", cw_workspace, iar_workspace)
	replicate_example("basic_rci", cw_workspace, iar_workspace)
	copy_rci_files("basic_rci", cw_workspace, iar_workspace)


	
	add_ewdemo_files_to_iar_project(iar_workspace + "\\" + "ewdemo\\ewdemo.ewp")
	add_basic_rci_files_to_iar_project(iar_workspace + "\\" + "basic_rci\\basic_rci.ewp")


main()