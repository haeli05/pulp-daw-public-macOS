#!/bin/sh
# Packs dist/PULP into dist/PULP.dmg: PULP.app, a link to /Applications to drag it onto, and
# pulp_render. Run by "./build.sh installer" after "./build.sh". With PULP_SIGN_IDENTITY set the
# image is signed too; notarize it afterwards with
#   xcrun notarytool submit dist/PULP.dmg --keychain-profile <profile> --wait
#   xcrun stapler staple dist/PULP.dmg
set -e
cd "$(dirname "$0")/../.."
[ -d dist/PULP/PULP.app ] || { echo "dist/PULP/PULP.app is not there: run ./build.sh first"; exit 1; }

stage="$(mktemp -d)"
trap 'rm -rf "$stage"' EXIT
cp -R dist/PULP/PULP.app "$stage/"
[ -f dist/PULP/pulp_render ] && cp dist/PULP/pulp_render "$stage/"
ln -s /Applications "$stage/Applications"

rm -f dist/PULP.dmg
hdiutil create -volname "PULP" -srcfolder "$stage" -fs HFS+ -format UDZO -ov dist/PULP.dmg
if [ -n "$PULP_SIGN_IDENTITY" ]; then codesign --force --timestamp -s "$PULP_SIGN_IDENTITY" dist/PULP.dmg; fi
echo "Built dist/PULP.dmg"
