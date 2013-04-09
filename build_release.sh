#!/bin/bash
rc=0
rca=0
PRODUCT_NAME=connector
BASE_DIR=$WORKSPACE/$PRODUCT_NAME
OUTPUT_DIR=$WORKSPACE/output
GETTING_STARTED_GUIDE=90001345
RELEASE_NOTES=93000761
HTML_ZIP=40002854
TARBALL_NUMBER=40002853
TARBALL_NAME=${TARBALL_NUMBER}_${REVISION}
NOTES_NAME=${RELEASE_NOTES}_${REVISION}
HTML_NAME=${HTML_ZIP}_${REVISION}
TOOLS_DIR=${BASE_DIR}/tools

SAMPLES="compile_and_link
         connect_on_ssl
         connect_to_device_cloud
         device_request
         file_system
         firmware_download
         remote_config
         send_data
         simple_remote_config"

function cleanup () 
{
    ARCHIVE=${WORKSPACE}/archive
    if [ -d "${ARCHIVE}" ]; then
        echo ">> Archive Directory already exists, cleaning it."
        rm -rfv "${ARCHIVE}"/*
    else
        echo ">> Creating Archive Directory."
        mkdir -p "${ARCHIVE}"
    fi
    echo ">> Archiving critical files."
    cp -v "${OUTPUT_DIR}/${TARBALL_NAME}.tgz" "${ARCHIVE}/"
    cp -v "${OUTPUT_DIR}/${NOTES_NAME}.zip" "${ARCHIVE}/"
    cp -v "${OUTPUT_DIR}/${HTML_NAME}.zip" "${ARCHIVE}/"


    echo ">> Cleaning Up ${OUTPUT_DIR} and ${BASE_DIR}"
    rm -r "${OUTPUT_DIR}"
    rm -r "${BASE_DIR}"
}

function build_config_tool ()
{
    mkdir -p "${TOOLS_DIR}"  
    ant -f "${WORKSPACE}/tools/config/build.xml" -Ddist.dir="${TOOLS_DIR}"
}

python dvt/scripts/makegen.py all

# Create the output directory.
mkdir -p "${OUTPUT_DIR}"

build_config_tool

# Create the doxygen documentation
cd doxygen
doxygen
cd ..

# Move the HTML files into the docs directory
mkdir -p docs/html
cp -rf doxygen/html/* docs/html
cp doxygen/user_guide.html docs/

# Create a "${PRODUCT_NAME}" subdirectory which will be the root of the tarball.
echo ">> Creating ${BASE_DIR} and copying public and private directories to it."
mkdir -p "${BASE_DIR}"
cp -rf private "${BASE_DIR}"
cp -rf public "${BASE_DIR}"
cp -rf docs "${BASE_DIR}"
rm -rf "${BASE_DIR}/public/run/platforms/freescale"

# Get the name of the getting starting guides and see which one is newer
released_file=$(find /eng/store/released/90000000 -name "${GETTING_STARTED_GUIDE}"*.pdf)
pending_file=$(find /eng/store/pending/90000000 -name "${GETTING_STARTED_GUIDE}"*.pdf)

if [ -f $pending_file ]; then
    echo ">> Pulling Getting Started Guide from ${pending_file}"
    cp "${pending_file}" "${BASE_DIR}/GettingStarted.pdf"
elif [ -f $released_file ]; then
    echo ">> Pulling Getting Started Guide from ${released_file}"
    cp "${released_file}" "${BASE_DIR}/GettingStarted.pdf"
else
    echo ">> Cannot find ${GETTING_STARTED_GUIDE}*.pdf"
    exit(1)
fi

# Replace the version number in Readme.txt to match the Tag used to build
if [ $TAG != "" ]
  then
    echo ">> Setting Release notes header to ${NOTES_NAME} v${TAG} in ${BASE_DIR}/private/Readme.txt"
    sed -i 's/ _RELEASE_NOTES_PARTNO_/ '"$NOTES_NAME"'/g' "${BASE_DIR}/private/Readme.txt"
    sed -i 's/ v_CONNECTOR_SW_VERSION_/ v'"$TAG"'/g' "${BASE_DIR}/private/Readme.txt"
    echo ">> Setting Version to ${TAG} in ${BASE_DIR}/private/connector_info.h"
    sed -i 's/#define CONNECTOR_SW_VERSION \S*/#define CONNECTOR_SW_VERSION "'"$TAG"'"/g' "${BASE_DIR}/private/connector_info.h"
fi

# Replace the date in Readme.txt to match today's date
today=`date +"%B %d, %Y"`
echo ">> Setting Release Date to Today (${today}) in ${BASE_DIR}/private/Readme.txt"
sed -i 's/_RELEASE_DATE_/'"${today}"'/g' "${BASE_DIR}/private/Readme.txt"

# Generate a Makefile for each sample.

# Create the tarball
echo ">> Creating the Tarball ${OUTPUT_DIR}/${TARBALL_NAME}.tgz."
tar --exclude="${PRODUCT_NAME}"/public/test --exclude="${PRODUCT_NAME}"/public/dvt -czvf "${OUTPUT_DIR}/${TARBALL_NAME}.tgz" "${PRODUCT_NAME}"/

# Create the Release Notes
echo ">> Creating the Release notes ${OUTPUT_DIR}/${NOTES_NAME}.zip"
zip -jvl "${OUTPUT_DIR}/${NOTES_NAME}.zip" "${BASE_DIR}/private/Readme.txt"

# Create the HTML ZIP
echo ">> Creating the Documenation tree ${OUTPUT_DIR}/${HTML_NAME}.zip"
cd "${BASE_DIR}/docs"
zip -v "${OUTPUT_DIR}/${HTML_NAME}.zip" user_guide.html 
zip -vr "${OUTPUT_DIR}/${HTML_NAME}.zip" html/ 
cd "${WORKSPACE}"

# Delete the original directory
echo ">> Removing base dir ${BASE_DIR}."
rm -rf "${BASE_DIR}"

# Uncompress the tarball we just created and run our tests
echo ">> Uncompressing ${OUTPUT_DIR}/${TARBALL_NAME}.tgz."
tar -xf "${OUTPUT_DIR}/${TARBALL_NAME}.tgz"

cd "${BASE_DIR}"
python ../dvt/scripts/replace_str.py public/run/platforms/linux/config.c '#error' '//#error'
python ../dvt/scripts/replace_str.py public/run/samples/compile_and_link/Makefile 'c99' 'c89'

# Build all the IIK samples and platforms

echo ">> Building all samples."
cd public/run/samples

for sample in $SAMPLES
do
  echo ">> Building $sample"
  cd $sample
  make clean; make all
  rc=$?
  if [[ ${rc} != 0 ]]; then
    echo "++ Failed to build $sample, exiting."
    cleanup
    exit ${rc} 
  fi
  cd ../
done

cd ../../../../

if [[ "${PENDING}" == "true" ]]; then
    # If successfull push the tarball to pending, if PENDING environment variable is set to 1.
    echo ">> Copying the Tarball to Pending."
    cp -v "${OUTPUT_DIR}/${TARBALL_NAME}.tgz" /eng/store/pending/40000000
    cp -v "${OUTPUT_DIR}/${NOTES_NAME}.zip" /eng/store/pending/93000000
    cp -v "${OUTPUT_DIR}/${HTML_NAME}.zip" /eng/store/pending/40000000
fi

cleanup
exit $rc
