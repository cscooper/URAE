#!/usr/bin/python

# This will open a routes file, insert vehicle definitions, and set the different
# vehicles in the routes file randomly.
# The following vehicle properties are required

# id			The name of the vehicle type
# accel			The acceleration ability of vehicles of this type (in m/s^2)
# decel 		The deceleration ability of vehicles of this type (in m/s^2)
# sigma			Driver imperfection
# length 		The vehicle's length (in m)
# color 		This vehicle type's color
# vClass 		An abstract vehicle class (see below)
# emissionClass An abstract emission class (see below); default: "P_7_7"
# width 		The vehicle's width [m] (only used for drawing)


import pickle
import csv
import random
import sys

vehicleDefinitions = []
routeFile = []

def CreateVehicleDefinitionXML( vDef ):
	vDefStr = "<vType"
	for k in vDef.iterkeys():
		vDefStr = vDefStr + " " + k + "=\"" + vDef[k] + "\""
	vDefStr = vDefStr + "/>"
	return vDefStr

def LoadVehicleDefinitions( fname ):
	f = open( fname, "r" )
	r = csv.reader( f, delimiter=" " )

	for line in r:
		vehicleDef = {}
		vehicleDef["id"] 			= line[0]
		vehicleDef["accel"] 		= line[1]
		vehicleDef["decel"] 		= line[2]
		vehicleDef["sigma"] 		= line[3]
		vehicleDef["length"] 		= line[4]
		vehicleDef["color"] 		= line[5]
		vehicleDef["width"] 		= line[6]
		vehicleDefinitions.append( vehicleDef )

	f.close()
	r = None

	print "Loaded " + str(len(vehicleDefinitions)) + " vehicle definitions."


def LoadRoutes( fname ):
	f = open( fname, "r" )
	for line in f:
		if "<routes" in line:
			routeFile.append(line)
			for vDef in vehicleDefinitions:
				routeFile.append( "    "+CreateVehicleDefinitionXML( vDef )+"\n" )
		elif "<vehicle" in line:
			n = int( random.random() * len(vehicleDefinitions) )
			routeFile.append( line.replace(">"," type=\"" + vehicleDefinitions[n]["id"] + "\">") )
		else:
			routeFile.append(line)

	f.close()
	
	print "Loaded route file '" + fname + "'"

def UpdateRoutes( fname ):
	f = open( fname, "w" )
	for line in routeFile:
		f.write( line )
	f.close()

def main():
	if len(sys.argv) != 4:
		print "Specify vehicle definitions file, route file, and an output filename."
		sys.exit(1)

	LoadVehicleDefinitions( sys.argv[1] )
	LoadRoutes( sys.argv[2] )
	UpdateRoutes( sys.argv[3] )

	print "Done!"

if __name__ == "__main__":
	main()

