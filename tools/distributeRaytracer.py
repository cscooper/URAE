#!/usr/bin/python

import sys
import os
import subprocess
from optparse import *

#TODO: THIS NEEDS TO BE FAR MORE ROBUST THAN JUST RETURNING SUCCESS OR FAILURE!
def RunCommand(commandString):
	return ( subprocess.call( commandString, shell=True) == 0 )


def main( options ):
	# first make a temporary directory
	print "Creating temporary directory..."
	streetName = options.basename[options.basename.rfind("/")+1:len(options.basename)]
	tempDir = streetName + "-temp"
	RunCommand( "mkdir " + tempDir )

	# next copy the map data and Raytracer binary to the temporary directory
	print "Copying files..."
	RunCommand( "cp " + options.basename + "* " + tempDir+"/" )
	RunCommand( "cp ../bin/Raytracer " + tempDir+"/" )

	# generate the configuration file
	print "Generating configuration..."
	os.chdir( tempDir )
	rtCmd = "./Raytracer -g -b " + streetName + " -r " + str(options.raycount) + " -i " + str(options.increment) + " -c " + str(options.cores) + " -G " + str(options.rxGain) + " -N " + str(options.areaCount) + " -F config"
	if options.rsuDefFile:
		rtCmd += " -R ../" + options.rsuDefFile
	RunCommand( rtCmd )
	os.chdir( ".." )

	# create the raytracer mediator script
	f = open("raytracerSim.temp.sh","w")
	f.write("#!/bin/sh\n")
	f.write("cd " + tempDir + "\n")
	f.write("echo $1-$2\n")
	f.write("./Raytracer $1 $2\n")
	f.write("cp *-$2.urae.k /home/cluster/euclid_share/"+tempDir+"/\n")
	f.close()

	# now run the cluster script
	clusterCmd = "~/Documents/Programming/ICTRCluster/omnet_cluster.py -s " + tempDir + " -j config,0:"+str(options.areaCount-1) + " -a 1 -U raytracerSim.temp.sh -S . -R ." 
	if options.ignoreNodes:
		clusterCmd += " -i " + options.ignoreNodes
	RunCommand( clusterCmd )

	# get the list of files containing k-factor results
	riceData = []
	n = 0
	fileList = [f for f in os.listdir("./"+tempDir) if ".urae.k" in f]
	for fname in fileList:
		f = open( tempDir+"/"+fname, "r" )
		d = f.readlines()
		n += int(d[0])
		riceData += d[1:]
		f.close()
		os.remove(tempDir+"/"+fname)

	# merge the data into one file
	f = open( tempDir+"/"+streetName+".urae.k", "w" )
	f.write(str(n)+"\n")
	for d in riceData:
		f.write(d)

	f.close()

	# copy the file back to the base directory
	folderName = options.basename[0:options.basename.rfind("/")]
	RunCommand( "cp " + tempDir + "/" + streetName + ".urae.k " + folderName + "/" )

	# Now clean up
	RunCommand( "rm -r " + tempDir + " raytracerSim.temp.sh" )

	print "Complete."


if __name__ == "__main__":
	optParser = OptionParser()
	
	procGroup = OptionGroup(optParser, "Input Options", "Choose the various input parameters")
	procGroup.add_option("-b", "--basename", dest="basename", type='string', help="The base filename for the CORNER files")
	procGroup.add_option("-r", "--raycount", dest="raycount", type='int', default=256, help="The number of rays to use")
	procGroup.add_option("-i", "--increment", dest="increment", type='float', default=10, help="The distance between adjacent K-factor sample point")
	procGroup.add_option("-c", "--cores", dest="cores", type='int', default=2, help="Number of cores to use for raytracing")
	procGroup.add_option("-G", "--rxGain", dest="rxGain", type='float', default=1, help="The gain of the receiver antenna")
	procGroup.add_option("-N", "--areaCount", dest="areaCount", type='int', default=4, help="The number of areas to divide the map into")
	procGroup.add_option("-I", "--ignoreNodes", dest="ignoreNodes", type='string', help="Specify nodes on the cluster to ignore (comma delimited).")
	procGroup.add_option("-R", "--rsuDefFile", dest="rsuDefFile", type='string', help="Specify RSU definitions file.")
	optParser.add_option_group(procGroup)

	(options, args) = optParser.parse_args()

	if not options.basename:
		print "Need to specify a base filename for the CORNER data."
		sys.exit(-1)

	main(options)








