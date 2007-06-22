#!/bin/sh

# This script cleans all targets, then builds the projects for the server, the plugins and the application (sclang) projects, one after another.

# clean
echo "Cleaning scsynth..."
xcodebuild -project Synth.xcodeproj -target "All" clean || exit

echo "Cleaning plugins..."
xcodebuild -project Plugins.xcodeproj -target "All" clean || exit

echo "Cleaning sclang..."
xcodebuild -project Language.xcodeproj -target "All" clean || exit

echo "Clean Done."

# scsynth
echo "Building scsynth..."
xcodebuild -project Synth.xcodeproj -target "All" -configuration "Deployment" build || exit

# plugins
echo "Building plugins..."
xcodebuild -project Plugins.xcodeproj -target "All" -configuration "Deployment" build || exit

#sclang
echo "Building sclang..."
xcodebuild -project Language.xcodeproj -target "All" -configuration "Deployment" build || exit

echo "Done."