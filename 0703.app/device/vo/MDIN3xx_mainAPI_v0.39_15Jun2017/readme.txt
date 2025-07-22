-. 컴파일 옵션

   보내 드린 코드는 MDIN325/MDIN340/MDIN380겸용 코드이므로

   mdintype.h파일에 ‘#define       SYSTEM_USE_MDIN325A’를 활성화 해주셔야 합니다.


 

-. I2C 함수

   Mdini2c.c 파일 내에 MDINI2C_Write()와 MDINI2C_Read()함수의 내부를 사용하시는 CPU에 맞게 수정하시면 되겠습니다.

   이 함수 내에 간단한 I2C Cycle에 대한 설명도 있으므로 그대로 구현하시면 되겠습니다.

   I2C는 16-bit 모드입니다.

   그리고 I2C Multi-Write도 반드시 확인 하셔야 하는데요.

   예를 들어, MDINHIF_MultiWrite(MDIN_LOCAL_ID, 0x200, (PBYTE)MDIN_Deinter_Default, 256);

   이 후에 mdincoef.h내에 있는 MDIN_Deinter_Default[]값이 정상적으로 Write되었는지 확인 해 보시기 바랍니다.


 

-. 초기화

   App325a.c 파일에 초기화 함수가 있고 main()함수를 참고하시면 되겠습니다.

   CreateMDIN325VideoInstance()함수 내에서 입/출력 세팅을 하게 되어 있는데 아래 내용만 입/출력에 맞게 세팅 하시면 되겠습니다.

           // 입력

           stVideo.stSRC_a.frmt = VIDSRC_1920x1080i60;            // 1080i

           stVideo.stSRC_a.mode = MDIN_SRC_SEP422_8;      // BT.1120 16-bit. : YCbCr 20-bit 일 경우 MDIN_SRC_SEP422_10사용

           // 출력

           stVideo.stOUT_m.frmt = VIDOUT_1920x1080p60;     // 1080P60 Output

           stVideo.stOUT_m.mode = MDIN_OUT_RGB444_8;     // RGB Output

-. 테스트 방법

   1. I2C가 정상적으로 구현이 되었다고 생각 하시면 Burst Write하는 것 까지 확인해 주시기 바랍니다.

      예를 들어, mdincoef.c파일 내에 MDIN_Deinter_Default[] 레지스트 값들이 있는데 이 값들이 잘 쓰여지는지

      읽어 봐서 확인해 주시기 바랍니다.

   2. Output Test Pattern

      출력 테스트 패턴은 입력영상과 무관하게 출력이 되는데 MDIN3xx_SetOutTestPattern MDIN_OUT_TEST_COLOR); 를 사용하셔서

      DAC 출력이 정상적으로 나오는지 확인해 주시기 바랍니다.

   3. Input Test Pattern

      입력 테스트 패턴은 입력 영상과 무관하게 내부 세팅이 정상적인지 확인하기 위함입니다.

      이 때에 CLK_A로 입력되는 클럭만 사용하게 됩니다.

      MDIN3xx_SetSrcTestPattern(&stVideo, MDIN_IN_TEST_H_COLOR);를 사용하시면 되겠습니다.

   4. 입력 영상

      입력 테스트 패턴이 정상으로 나올 경우 실제 입력 영상을 확인하시면 되겠습니다.