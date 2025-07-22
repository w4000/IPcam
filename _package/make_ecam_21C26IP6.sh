#!/bin/bash


TOP_DIR=$PWD

APP_DIR=${TOP_DIR}/../0703.app
# APP_DIR=${TOP_DIR}/../240221_0.0.v8.app/0703.app
ONVIF_DIR=${TOP_DIR}/../onvif
ONVIF_BIN=${ONVIF_DIR}/bin/hi
DEFINE_DIR=${TOP_DIR}/define

# 파일 경로 설정
VERSION_FILE=${APP_DIR}/version

TEMP_DIR=${TOP_DIR}/.temp
ROOTFS=rootfs
ROOTFS_HOME=rootfs_home



usage()
{
	echo ""
	echo "Usage : make_ecam.sh [VERSION] "
	echo ""
	echo "[VERSION]      : Firmware version.(N.N.N format)"
	echo ""
	exit 1
}

if [ "$#" -lt 1 ]; then
	usage
fi


# A.B.C 형식의 버전 정보를 변수에 할당
IFS='.' read -r MK_SYS_VER1 MK_SYS_VER2 MK_SYS_VER3 <<< "$1"

# 파일 내용 업데이트
sed -i "s/^MK_SYS_VER1=.*/MK_SYS_VER1=${MK_SYS_VER1}/" "$VERSION_FILE"
sed -i "s/^MK_SYS_VER2=.*/MK_SYS_VER2=${MK_SYS_VER2}/" "$VERSION_FILE"
sed -i "s/^MK_SYS_VER3=.*/MK_SYS_VER3=${MK_SYS_VER3}/" "$VERSION_FILE"

echo "파일 내용이 업데이트되었습니다: $VERSION_FILE"

# echo "Revision:$REV $#"
MAKERELEASE=1
SENSOR=IP6


YEAR=`date +%y`
YEAR=`expr ${YEAR} - 24`
DATE_VERSION=${YEAR}`date +.%m.%d`



# VERSION=${DATE_VERSION}

VERSION=$1



FIRMWARE_FILE_PREFIX=21C26
MODEL=${FIRMWARE_FILE_PREFIX}

NARCH=NT98566${SDK}
NCHIP=98566
PRE_IMAGE=image
SENSOR_TYPE=imx327
MODEL+=${SENSOR}


IPDEVICE_TYPE=327
SUB_MODEL=0

MODEL_PATH=${MODEL}
OUTPUT_PATH=${TOP_DIR}/_output_nova/${MODEL_PATH}
OUTPUT_FILE="${MODEL}_v${VERSION}.bin"
IMAGE_PATH=${OUTPUT_PATH}/images/
SDCARD_PATH=${OUTPUT_PATH}/sdcard
PREBURN_PATH=${OUTPUT_PATH}/preburn
DEVICE_VER1=${VERSION%%.*}
DEVICE_VER_TMP=${VERSION#*.}
DEVICE_VER2=${DEVICE_VER_TMP%%.*}
DEVICE_VER_TMP1=${DEVICE_VER_TMP#*.}
DEVICE_VER3=${DEVICE_VER_TMP1%%.*}
DEVICE_VER4=1
DEVICE_VERSION=${DEVICE_VER1}.${DEVICE_VER2}.${DEVICE_VER3}.${DEVICE_VER4}

echo "DEVICE_VER1 >" ${DEVICE_VER1}
echo "DEVICE_VER2 >" ${DEVICE_VER2}
echo "DEVICE_VER3 >" ${DEVICE_VER3}
echo "DEVICE_VERSION >" ${DEVICE_VERSION}
echo "VERSION > " ${VERSION}



MAKE_OPT="REL_BUILD=1 MK_CAM_RELEASE=1 MODEL=${MODEL} NARCH=${NARCH} SENSOR_TYPE=${SENSOR_TYPE}  SDK_VER=${SDK} DEVICE_VER1=${DEVICE_VER1} DEVICE_VER2=${DEVICE_VER2} DEVICE_VER3=${DEVICE_VER3}"
make -C ${APP_DIR} -j all_with_tools ${MAKE_OPT}




if [ -d ${TEMP_DIR} ]; then
	rm ${TEMP_DIR} -rf
fi


mkdir ${TEMP_DIR}
tar zxvf ${TOP_DIR}/skel/${NARCH}/rootfs.tgz -C ${TEMP_DIR}/
cp ${TOP_DIR}/prebuild/${NARCH}/rootfs/* ${TEMP_DIR}/rootfs -a
cp ${TOP_DIR}/prebuild/${NARCH}/${ROOTFS_HOME} ${TEMP_DIR} -a

make -C ${APP_DIR} install DEST_DIR=${TEMP_DIR}/${ROOTFS_HOME} ${MAKE_OPT}


UBINIZE=ubinize
NVTLDOP=${TOP_DIR}/tools/nvt-ld-op
NVTPACK=${TOP_DIR}/tools/nvtpack
UBI_BLK_SIZE="128KiB"		


gen_rootfs_ubinize_cfg_param()
{
	echo -e "[ubifs]"			> ${2}
	echo -e "mode=ubi"			>> ${2}
	echo -e "image=${1}"		>> ${2}
	echo -e "vol_id=0"			>> ${2}
	echo -e "vol_size=${3}"		>> ${2}
	echo -e "vol_type=dynamic"	>> ${2}
	echo -e "vol_name=${4}"		>> ${2}
}

gen_nvtpack_ini()
{
	echo -e "[NVTPACK_FW_INI_16072017]"		> ${1}/${2}
	echo -e "GEN ${5}"						>> ${1}/${2}
	echo -e "CHIP_SELECT 1"					>> ${1}/${2}
	echo -e "ITEM00 0"						>> ${1}/${2}
	echo -e "ITEM01 1 ${1}/nvt-na51055-evb.bin"	>> ${1}/${2}
	echo -e "ITEM02 0"							>> ${1}/${2}
	echo -e "ITEM03 1 ${1}/u-boot_${NARCH}.bin"	>> ${1}/${2}
	echo -e "ITEM04 0"							>> ${1}/${2}
	echo -e "ITEM05 1 ${1}/uImage_${NARCH}"	>> ${1}/${2}
	echo -e "ITEM06 1 ${1}/${3}"				>> ${1}/${2}
	echo -e "ITEM07 1 ${1}/${4}"				>> ${1}/${2}
	echo -e "ITEM08 0"						>> ${1}/${2}
	echo -e "ITEM09 0"						>> ${1}/${2}
	echo -e "ITEM10 0"						>> ${1}/${2}
	echo -e "ITEM11 0"						>> ${1}/${2}
	echo -e "ITEM12 0"						>> ${1}/${2}
	echo -e "ITEM13 0"						>> ${1}/${2}
	echo -e "ITEM14 0"						>> ${1}/${2}
	echo -e "ITEM15 0"						>> ${1}/${2}
	echo -e "ITEM16 0"						>> ${1}/${2}
	echo -e "ITEM17 0"						>> ${1}/${2}
	echo -e "ITEM18 0"						>> ${1}/${2}
	echo -e "ITEM19 0"						>> ${1}/${2}
	echo -e "ITEM20 0"						>> ${1}/${2}
	echo -e "ITEM21 0"						>> ${1}/${2}
	echo -e "ITEM22 0"						>> ${1}/${2}
	echo -e "ITEM23 0"						>> ${1}/${2}
	echo -e "ITEM24 0"						>> ${1}/${2}
	echo -e "ITEM25 0"						>> ${1}/${2}
	echo -e "ITEM26 0"						>> ${1}/${2}
	echo -e "ITEM27 0"						>> ${1}/${2}
	echo -e "ITEM28 0"						>> ${1}/${2}
	echo -e "ITEM29 0"						>> ${1}/${2}
	echo -e "ITEM30 0"						>> ${1}/${2}
	echo -e "ITEM31 0"						>> ${1}/${2}
}

MKFIRMWARE=${TOP_DIR}/tools/mkfirmware
ROOTFS_IMAGE=rootfs_${MODEL}.ubifs
IPM_IMAGE=ipm.ubifs
UBOOT_IMAGE=${TOP_DIR}/prebuild/${NARCH}/${PRE_IMAGE}/u-boot_NT98566.bin
KERNEL_IMAGE=${TOP_DIR}/prebuild/${NARCH}/${PRE_IMAGE}/uImage_NT98566
DT_IMAGE=${TOP_DIR}/prebuild/${NARCH}/${PRE_IMAGE}/nvt-na51055-evb.bin
FIRMWARE_IMAGE=${OUTPUT_PATH}/${OUTPUT_FILE}
FW_IMAGE=${SDCARD_PATH}/FW98566A.bin
NVTPACK_INI=FW${NCHIP}A.ini


if [ ! -d ${OUTPUT_PATH} ]; then
	mkdir -p ${OUTPUT_PATH}
fi

if [ ! -d ${IMAGE_PATH} ]; then
	mkdir -p ${IMAGE_PATH}
fi

if [ ! -d ${SDCARD_PATH} ]; then
	mkdir -p ${SDCARD_PATH}
fi

TEMP_ROOTFS=${TEMP_DIR}/${ROOTFS}
TEMP_ROOTFS_HOME=${TEMP_DIR}/${ROOTFS_HOME}
TEMP_IPM=${TEMP_DIR}/ipm


mksquashfs ${TEMP_ROOTFS_HOME} ${TEMP_ROOTFS}/rootfs_home.squashfs -b 128K -comp xz
mkfs.ubifs --squash-uids -x lzo -m 2048  -e 126976 -c 476 -o ${ROOTFS_IMAGE}.raw -r ${TEMP_ROOTFS}
gen_rootfs_ubinize_cfg_param ${ROOTFS_IMAGE}.raw ubinize.cfg 60440576 rootfs	#60440576 ? 126976*leb
${UBINIZE} -m 2048 -p ${UBI_BLK_SIZE} -s 2048 -o ${ROOTFS_IMAGE}.ubi.raw ubinize.cfg
${NVTLDOP} --packsum-src=${ROOTFS_IMAGE}.ubi.raw --packsum-dst=${ROOTFS_IMAGE}.cs --packsum-type=0x9

mkdir -p ${TEMP_IPM}
mkfs.ubifs --squash-uids -x lzo -m 2048 -e 126976 -c 388 -o ${IPM_IMAGE}.raw -r ${TEMP_IPM}
gen_rootfs_ubinize_cfg_param ${IPM_IMAGE}.raw ubinize.cfg 49266688 app    #48885760 ? 126976*leb
${UBINIZE} -m 2048 -p ${UBI_BLK_SIZE} -s 2048 -o ${IPM_IMAGE}.ubi.raw ubinize.cfg
${NVTLDOP} --packsum-src=${IPM_IMAGE}.ubi.raw --packsum-dst=${IPM_IMAGE}.cs --packsum-type=0x9
dd if=${ROOTFS_IMAGE}.cs of=${ROOTFS_IMAGE} skip=1 bs=64
${MKFIRMWARE} ${ROOTFS_IMAGE} ${FIRMWARE_IMAGE} 


cp ${UBOOT_IMAGE} ${IMAGE_PATH}/u-boot_${NARCH}.bin
cp ${KERNEL_IMAGE} ${IMAGE_PATH}/uImage_${NARCH}
cp ${DT_IMAGE} ${IMAGE_PATH}/nvt-na51055-evb.bin
cp ${ROOTFS_IMAGE} ${IMAGE_PATH}
cp ${ROOTFS_IMAGE}.cs ${IMAGE_PATH}/rootfs.ubifs
cp ${IPM_IMAGE}.cs ${IMAGE_PATH}/ipm.ubifs

gen_nvtpack_ini ${IMAGE_PATH} ${NVTPACK_INI} rootfs.ubifs ipm.ubifs ${FW_IMAGE}
${NVTPACK} -fw ${IMAGE_PATH}/${NVTPACK_INI}


echo "Firmware: ${FIRMWARE_IMAGE}"

rm ${ROOTFS_IMAGE} -rf
rm ${ROOTFS_IMAGE}.raw -rf
rm ${ROOTFS_IMAGE}.ubi.raw -rf
rm ${ROOTFS_IMAGE}.cs -rf
rm ${IPM_IMAGE} -rf
rm ${IPM_IMAGE}.raw -rf
rm ${IPM_IMAGE}.ubi.raw -rf
rm ${IPM_IMAGE}.cs -rf
rm ubinize.cfg
rm ${TEMP_DIR} -rf 