#!/bin/bash

# 오늘 날짜를 YYYYMMDD 형식으로 저장
TODAY=$(date +%Y%m%d)

# 현재 위치에서 시작
# _package 폴더로 이동
cd _package || { echo "_package 폴더를 찾을 수 없습니다."; exit 1; }

# make_clean.sh 실행
if [ -x "./make_clean.sh" ]; then
    ./make_clean.sh
else
    echo "make_clean.sh 실행 권한이 없거나 파일이 없습니다."
    exit 1
fi

# 상위 폴더로 이동
cd ..

# _package 폴더를 이름에 날짜를 포함하여 압축
tar -czf "_package_${TODAY}.tgz" _package

# 0703.app 폴더를 날짜를 포함하여 압축
if [ -d "0703.app" ]; then
    tar -czf "0703.app_${TODAY}.tgz" "0703.app"
else
    echo "0703.app 폴더가 존재하지 않습니다."
    exit 1
fi

echo "작업이 완료되었습니다: _package_${TODAY}.tgz 와 0703.app_${TODAY}.tgz"
