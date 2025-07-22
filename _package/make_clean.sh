#!/bin/bash

TOP_DIR=$PWD
APP_DIR=${TOP_DIR}/../0703.app
ONVIF_SERVER_DIR=${APP_DIR}/onvifserver
ONVIF_SERVER_ORG_DIR=${TOP_DIR}/onvifserver_org  # TOP_DIR 하위에 있는 onvifserver_org 경로

# 모든 .o 파일 삭제
echo "모든 .o 파일 삭제 시작..."
find ${APP_DIR} -type f -name "*.o" -exec rm -f {} +
echo "모든 .o 파일이 ${APP_DIR}에서 삭제되었습니다."

# 모든 .cgi 파일 삭제
echo "모든 .cgi 파일 삭제 시작..."
find ${APP_DIR} -type f -name "*.cgi" -exec rm -f {} +
echo "모든 .cgi 파일이 ${APP_DIR}에서 삭제되었습니다."

# 모든 .fcgi 파일 삭제
echo "모든 .fcgi 파일 삭제 시작..."
find ${APP_DIR} -type f -name "*.fcgi" -exec rm -f {} +
echo "모든 .fcgi 파일이 ${APP_DIR}에서 삭제되었습니다."

# 모든 .so 파일 삭제
#echo "모든 .so 파일 삭제 시작..."
#find ${APP_DIR} -type f -name "*.so" -exec rm -f {} +
#echo "모든 .so 파일이 ${APP_DIR}에서 삭제되었습니다."

echo "output 폴더 삭제"
rm -rf ./_output_nova

# onvifserver 폴더 생성 (존재하지 않는 경우)
#echo "onvifserver 폴더를 생성합니다... 경로: ${ONVIF_SERVER_DIR}"
#mkdir -p ${ONVIF_SERVER_DIR}
#echo "onvifserver 폴더가 생성되었습니다: ${ONVIF_SERVER_DIR}"

# onvifserver_org 폴더의 내용을 onvifserver로 복사
#echo "onvifserver_org 폴더의 내용을 onvifserver로 복사합니다... 원본 경로: ${ONVIF_SERVER_ORG_DIR}, 대상 경로: ${ONVIF_SERVER_DIR}"
#cp -rf ${ONVIF_SERVER_ORG_DIR}/* ${ONVIF_SERVER_DIR}/
#echo "onvifserver_org의 내용이 onvifserver로 복사되었습니다: ${ONVIF_SERVER_DIR}"

echo "스크립트 실행 완료."