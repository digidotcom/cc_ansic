import shutil
from tempfile import mkstemp
import os
from glob import glob
import time
import sys

# Where the IAR template project is and where the created examples will be
iar_workspace_leading = "C:\Etherios4Kinetis\Projects\uCOS\Micrium\Software\EvalBoards\Freescale\TWR-K"
iar_workspace_trail = "N512\IAR\etherios_projects"  	
# The template application name (and folder)
template_name = "etherios_app" 					

# Customize here your development preferences that will be set to connector_config.h if this script is 
# called with "DEV" as first argument
dev 	   = 0
dev_mac    = "00:50:C2:25:60:02"
dev_cloud  = "login.etherios.co.uk"
dev_vendor = "0x04000036"


# Where the GIT repository is
BASE_DIR = os.getcwd() + "\\..\\..\\.."

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


# Renames template_project.ewp by the new_name.ewp, customize CPU and replaces the reference in the IAR workspace (.eww)
def change_iar_project_name(sample_dir, new_name, tower):
	#sample_dir is the sample's root
	replace(sample_dir + "\etherios_app.eww", 
			"<path>$WS_DIR$\etherios_app.ewp</path>", "<path>$WS_DIR$\\" + new_name + ".ewp</path>")
	replace(sample_dir + "\etherios_app.ewp", 
			"<state>etherios_app.srec</state>", "<state>" + new_name + ".srec</state>")
	replace(sample_dir + "\etherios_app.ewp", 
			"<state>etherios_app.out</state>", "<state>" + new_name + ".out</state>")

	# Replace Target Chip based on tower used
	if tower == "53":
		replace(sample_dir + "\etherios_app.ewp", 
			"<state>__RELACE_WITH__CHIP__</state>", "<state>MK53DN512Zxxx10	Freescale MK53DN512Zxxx10</state>")
	else:
		replace(sample_dir + "\etherios_app.ewp", 
			"<state>__RELACE_WITH__CHIP__</state>", "<state>MK60DN512Zxxx10	Freescale MK60DN512Zxxx10</state>")


	os.rename(sample_dir + "\etherios_app.ewp", sample_dir + "\\" + new_name + ".ewp")
	os.rename(sample_dir + "\etherios_app.eww", sample_dir + "\\" + new_name + ".eww")
	os.rename(sample_dir + "\etherios_app.ewd", sample_dir + "\\" + new_name + ".ewd")

# This hard-coded function adds the RCI files to BASIC RCI project.
def add_basic_rci_files_to_iar_project(iar_ewp_file, tower):
	iar_ewp_file_with_path = iar_workspace_leading + tower + iar_workspace_trail + "\\" + iar_ewp_file  
	files_to_add = ("gps_stats.c", "system.c", "remote_config.h", "remote_config_cb.h", "remote_config_cb.c")
	file_tag = "\n\n<file>\n"
	file_tag_close = "\n\n</file>\n"
	name_tag = "\n\n<name>"
	name_tag_close = "</name>\n"
	#Create temp file
	fh, abs_path = mkstemp()
	new_file = open(abs_path,'w')
	old_file = open(iar_ewp_file_with_path)
	for line in old_file:
		new_file.write(line)
		if "<name>APP</name>" in line:
			for file in files_to_add:
				full_file_path = "$PROJ_DIR$\\" + file
				new_file.write(file_tag + name_tag + full_file_path + name_tag_close + file_tag_close)
	#close temp file
	new_file.close()
	os.close(fh)
	old_file.close()
	#Remove original file
	os.remove(iar_ewp_file_with_path)
	#Move new file
	shutil.move(abs_path, iar_ewp_file_with_path)

# Exclude from build not required uCOS components for an example
def add_exclude(example_name, tower, name):
	iar_workspace = iar_workspace_leading + tower + iar_workspace_trail

	sample_iar = iar_workspace + "\\" + example_name + "\\" + example_name + ".ewp"

	replace(sample_iar, 
			"<name>" + name + "</name>", 
			"<name>" + name + "</name>\n" + 
			"    <excluded>\n" + 
			"      <configuration>Release</configuration>\n" +
			"      <configuration>Flash</configuration>\n" + 
			"    </excluded>")

# Replaces some configuration parameters in connector_config.h for development
def customize_params_development(sample_dir):
	#sample_dir is the sample's root
	replace(sample_dir + "\connector_config.h", 
			"//#define CONNECTOR_MAC_ADDRESS", "#define CONNECTOR_MAC_ADDRESS")
	replace(sample_dir + "\connector_config.h", 
			"00:00:00:00:00:00", dev_mac)

	replace(sample_dir + "\connector_config.h", 
			"//#define CONNECTOR_CLOUD_URL", "#define CONNECTOR_CLOUD_URL")
	replace(sample_dir + "\connector_config.h", 
			"login.etherios.com", dev_cloud)

	replace(sample_dir + "\connector_config.h", 
			"//#define CONNECTOR_VENDOR_ID", "#define CONNECTOR_VENDOR_ID")
	replace(sample_dir + "\connector_config.h", 
			"0x00000000", dev_vendor)

# This functions copies the template application, changes its name by "example_name" and replaces demo-specific files from GIT repository
def replicate_example(example_name, tower):
	iar_workspace = iar_workspace_leading + tower + iar_workspace_trail

	template_dir_iar = iar_workspace + "\\" + template_name
	dest_sample_dir_iar = iar_workspace + "\\" + example_name
	if os.path.exists(dest_sample_dir_iar):
		shutil.rmtree(dest_sample_dir_iar)
	shutil.copytree(template_dir_iar, dest_sample_dir_iar)

	for file in glob(BASE_DIR + "\kits\kinetis\ucos\samples\\" + example_name  + "\*.*"):
		shutil.copy(file, dest_sample_dir_iar + "")


	change_iar_project_name(dest_sample_dir_iar, example_name, tower)

	if dev == 1:
		customize_params_development(dest_sample_dir_iar)

# This function deletes template application
def delete_example(tower):
	dir_path = iar_workspace_leading + tower + iar_workspace_trail + "\\etherios_app"
	#os.remove(dir_path)
	shutil.rmtree(dir_path)

# main application 
def main():
	global dev
	print 'git repo: %s' %(BASE_DIR)

	if len(sys.argv) > 1 and sys.argv[1] == "DEV":
		dev = 1

	replicate_example("connect_to_etherios", "53")
	add_exclude("connect_to_etherios", "53", "uC/FS")
	add_exclude("connect_to_etherios", "53", "uC/Clk")
	replicate_example("connect_to_etherios", "60")
	add_exclude("connect_to_etherios", "60", "uC/FS")
	add_exclude("connect_to_etherios", "60", "uC/Clk")

	replicate_example("send_data", "53")
	add_exclude("send_data", "53", "uC/FS")
	add_exclude("send_data", "53", "uC/Clk")
	replicate_example("send_data", "60")
	add_exclude("send_data", "60", "uC/FS")
	add_exclude("send_data", "60", "uC/Clk")

	replicate_example("device_request", "53")
	add_exclude("device_request", "53", "uC/FS")
	add_exclude("device_request", "53", "uC/Clk")
	replicate_example("device_request", "60")
	add_exclude("device_request", "60", "uC/FS")
	add_exclude("device_request", "60", "uC/Clk")

	#file_system sample requires uC/FS and uC/Clk
	replicate_example("file_system", "53")
	replicate_example("file_system", "60")

	replicate_example("firmware_update", "53")
	add_exclude("firmware_update", "53", "uC/FS")
	add_exclude("firmware_update", "53", "uC/Clk")
	replicate_example("firmware_update", "60")
	add_exclude("firmware_update", "60", "uC/FS")
	add_exclude("firmware_update", "60", "uC/Clk")

	replicate_example("basic_rci", "53")
	add_exclude("basic_rci", "53", "uC/FS")
	add_exclude("basic_rci", "53", "uC/Clk")
	replicate_example("basic_rci", "60")
	add_exclude("basic_rci", "60", "uC/FS")
	add_exclude("basic_rci", "60", "uC/Clk")

	#data_points sample uC/Clk
	replicate_example("data_points", "53")
	add_exclude("data_points", "53", "uC/FS")
	replicate_example("data_points", "60")
	add_exclude("data_points", "60", "uC/FS")

	delete_example("53")
	delete_example("60")
	
#	add_basic_rci_files_to_iar_project("basic_rci\\basic_rci.ewp", "53")
#	add_basic_rci_files_to_iar_project("basic_rci\\basic_rci.ewp", "60")
main()