#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>

#include "actuators/dotmatrix.h"
#include "actuators/lcd.h"
#include "sensors/dht11.h"
#include "sensors/soilmoisture.h"
#include "sensors/photoresistor.h"
#include "sensors/motionsensor.h"
#include "utility/send_localhost_text.h"
#include "utility/delay.h"
#include "communication/tts.h"

// 테스트 설정 구조체
typedef struct
{
    int fast_delay;              // 밀리초 단위, 0이면 fast mode 비활성화
    int sensitive_mode;          // 1이면 민감한 임계값 사용
    int always_check_brightness; // 1이면 낮/밤 관계없이 밝기 체크
    int enable_motionsensor;
    int enable_temperature;
    int enable_soilmoisture;
    int enable_brightness;
} Config;

// 임계값 구조체
typedef struct
{
    float temp_lower;
    float temp_upper;
    float soil_dry;
} Thresholds;

// 일반 모드 임계값
const Thresholds normal_thresholds = {
    .temp_lower = 16.0,
    .temp_upper = 30.0,
    .soil_dry = 20.0};

// 민감 모드 임계값
const Thresholds sensitive_thresholds = {
    .temp_lower = 18.0,
    .temp_upper = 28.0,
    .soil_dry = 25.0};

// 옵션 설정
Config config = {
    .fast_delay = 0,
    .sensitive_mode = 0,
    .always_check_brightness = 0, // 기본적으로는 낮/밤 구분
    .enable_motionsensor = 1,
    .enable_temperature = 1,
    .enable_soilmoisture = 1,
    .enable_brightness = 1};

// Current thresholds (normal_thresholds 또는 sensitive_thresholds 중 하나를 가리킴)
const Thresholds *current_thresholds = &normal_thresholds;

char status_temperature[16] = ""; // 현재 식물 상태 저장 배열 전역변수
char status_soilmoisture[16] = "";
char status_brightness[16] = "";

void print_usage()
{
    printf("Usage: ./program [OPTIONS]\n");
    printf("Options:\n");
    printf("  --fast=MSEC       빠른 모드 활성화 (밀리초 단위 딜레이)\n");
    printf("  --sensitive       민감 모드 활성화 (더 엄격한 임계값 적용)\n");
    printf("  --24h-bright     밝기를 24시간 체크 (기본: 낮 시간만)\n");
    printf("  --no-motion       모션 센서 비활성화\n");
    printf("  --no-temp         온도 센서 비활성화\n");
    printf("  --no-soil         토양습도 센서 비활성화\n");
    printf("  --no-bright       밝기 센서 비활성화\n");
    printf("  -h, --help        도움말 출력\n");
}

// 옵션 파싱 함수
void parse_arguments(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"fast", required_argument, 0, 'f'},
        {"sensitive", no_argument, 0, 's'},
        {"24h-bright", no_argument, 0, 'a'},
        {"no-motion", no_argument, 0, 'm'},
        {"no-temp", no_argument, 0, 't'},
        {"no-soil", no_argument, 0, 'o'},
        {"no-bright", no_argument, 0, 'b'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c;

    while ((c = getopt_long_only(argc, argv, "h", long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'f':
            config.fast_delay = atoi(optarg);
            break;
        case 's':
            config.sensitive_mode = 1;
            current_thresholds = &sensitive_thresholds;
            break;
        case 'a':
            config.always_check_brightness = 1;
            break;
        case 'm':
            config.enable_motionsensor = 0;
            break;
        case 't':
            config.enable_temperature = 0;
            break;
        case 'o':
            config.enable_soilmoisture = 0;
            break;
        case 'b':
            config.enable_brightness = 0;
            break;
        case 'h':
            print_usage();
            exit(0);
        case '?':
            print_usage();
            exit(1);
        }
    }
}

typedef enum
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void log_sensor(const char *sensor_type, LogLevel level,
                float value, const char *unit,
                const char *message,
                float min_threshold, float max_threshold)
{
    time_t now;
    struct tm *local;
    char timestamp[20];

    time(&now);
    local = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", local);

    const char *level_str = "INFO";
    const char *level_symbol = "•";
    if (level == LOG_WARNING)
    {
        level_str = "WARN";
        level_symbol = "⚠️";
    }
    else if (level == LOG_ERROR)
    {
        level_str = "ERROR";
        level_symbol = "❌";
    }

    printf("\n%s [%s] %s %s\n", timestamp, level_str, level_symbol, sensor_type);
    printf("측정값: %.1f%s (허용범위: %.1f-%.1f%s)\n",
           value, unit, min_threshold, max_threshold, unit);
    if (message)
    {
        printf("상태: %s\n", message);
    }
}

// (만약 음성 출력까지 구현했을 때) 주기적 측정과 순간적 측정 타이밍이 겹칠 경우, 두 케이스에 대한 모든 음성 출력이 겹칠 텐데 이를 어떻게 처리할 것인가.

// 토양습도 측정 주기의 경우, 점토질 토양에 물을 줄 때 완전히 스며들어 없어지는 시간까지 평균적으로 2~3일이므로 넉넉하게 24시간에 한 번씩
// 측정해도 될 것이라고 판단.

// 채소 및 과일: 41% - 80% (예: 딸기, 토마토 등)
// 다육식물 및 선인장: 20% - 40% (건조한 환경에 적합)
// 관엽식물: 21% - 60% (종류에 따라 다름)

// 시간별 (토양)습도 변화
// 다음은 점토질 토양에서 물을 흡수한 후 시간별로 예상되는 습도 변화입니다
// 시간 경과	토양 습도 (%)	설명
// 0~1시간	20~30%	초기 건조 상태. 물이 표면에서 빠르게 흡수되기 시작.
// 1~3시간	30~50%	물이 점차 깊은 층으로 스며들며, 습도가 증가.
// 3~6시간	50~70%	흙의 중간 층까지 물이 도달하며, 점토가 포화 상태에 가까워짐.
// 6~24시간	70~90%	거의 포화 상태에 도달하며, 잔류 수분이 천천히 이동.
// 24시간 이후	90~100%	포화 상태에 도달. 더 이상의 물 흡수는 제한적이며 배수가 시작될 수 있음.
float abs_fl(float num1, float num2) // | num1 - num2 |에 대한 절댓값을 구하는 함수
{
    if ((num1 - num2) < 0)
    {
        return -(num1 - num2);
    }

    else if ((num1 - num2) > 0)
    {
        return (num1 - num2);
    }

    else // num1 - num2 = 0인 경우
    {
        return 0;
    }
}

float detect_noise(float value) // 센서값이 튀는 것을 감지하여 적절한 값으로 설정해주는 함수, value : 센서의 측정 값
{
    float current_temp = get_temperature();
    float second_temp;

    if (abs_fl(value, current_temp) > 5.0)
    {
        second_temp = get_temperature();

        if (abs_fl(current_temp, second_temp) < 1.0)
        {
            return current_temp;
        }

        else
        {
            return second_temp;
        }
        // 측정한 온도 값이 이전 값과 5도 이상 차이 날 경우, 다시 온도를 측정한다.
        // 만약 다시 측정한 값도 기존 값과 비슷한 차이를 보일 경우, 그 값을 적용한다. = 튄 것이 아닌 환경 변화로 인한 급격한 오도 변화로 판단
        // 측정한 온도 값(첫 번째 측정한 값(current_temp))이 튄 값으로 판단될 경우, 한 번 더 측정한 값으로 업데이트해준다.
    }

    else
    {
        return current_temp; // 기존 온도값과 측정 온도값이 5도 이하의 차이라면 그 값으로 온도값을 업데이트 한다.
    }
    // case 1: //토양습도센서 (12시간 단위 측정을 기준으로 만듬)
    //     current_soilmost = *(float*)value;
    //     if(abs(soilmoisture - current_soilmost) > 25.0){ //12시간 후 측정 습도가 25%차 라면
    //     }
    //     else{
    //     }
    // 구현하려고 하였으나 토양습도센서의 경우, 값이 튀는 경우가 없어 그냥 뺐다.
}

void *t_temperature()
{
    static float prev_temperature = 22.5;
    float temperature = 22.5;

    while (1)
    {
        temperature = detect_noise(prev_temperature); // 튀는 값 잡고, 전역변수에 온도값 반영.

        if (temperature <= current_thresholds->temp_lower)
        {
            strcpy(status_temperature, "Cold!");
            log_sensor("온도", LOG_WARNING, temperature, "°C",
                       "너무 춥습니다. 따뜻한 곳으로 옮겨주세요.",
                       current_thresholds->temp_lower,
                       current_thresholds->temp_upper);
            tts_talk("여기는 너무 추워요. 따뜻한 곳으로 옮겨주세요.");
        }

        else if (temperature >= current_thresholds->temp_upper)
        {
            strcpy(status_temperature, "Hot!");
            log_sensor("온도", LOG_WARNING, temperature, "°C",
                       "너무 덥습니다. 시원한 곳으로 옮겨주세요.",
                       current_thresholds->temp_lower,
                       current_thresholds->temp_upper);
            tts_talk("여기는 너무 더워요. 시원한 곳으로 옮겨주세요.");
        }
        else
        {
            strcpy(status_temperature, "");
            log_sensor("온도", LOG_INFO, temperature, "°C", NULL,
                       current_thresholds->temp_lower,
                       current_thresholds->temp_upper);
        }

        prev_temperature = temperature;

        if (config.fast_delay)
            delay(config.fast_delay);
        else
            delay_hour(1);
    }
}

void *t_soilmoisture()
{
    float soilmoisture;

    while (1)
    {
        soilmoisture = get_soilmoisture();

        if (soilmoisture <= current_thresholds->soil_dry)
        {
            strcpy(status_soilmoisture, "Dry!");
            log_sensor("토양습도", LOG_WARNING, soilmoisture, "%",
                       "토양이 너무 건조합니다. 물을 주세요.",
                       current_thresholds->soil_dry, 100.0);
            tts_talk("흙이 너무 건조해요. 저에게 물을 주세요.");
        }
        else
        {
            strcpy(status_soilmoisture, "");
            log_sensor("토양습도", LOG_INFO, soilmoisture, "%", NULL,
                       current_thresholds->soil_dry, 100.0);
        }

        if (config.fast_delay)
            delay(config.fast_delay);
        else
            delay_hour(12); // 12시간마다 토양습도 측정, 테스트를 위해 시간을 짧게 만들어 놓음.
    }
}

void *t_brightness()
{
    int is_bright;
    time_t now;
    struct tm *local_time;

    while (1)
    {
        is_bright = get_brightness();
        time(&now);
        local_time = localtime(&now);

        // 항상 체크하거나 낮 시간일 때만 체크
        int should_check = config.always_check_brightness ||
                           (local_time->tm_hour >= 7 && local_time->tm_hour < 19);

        if (!is_bright && should_check)
        {
            strcpy(status_brightness, "Dark!");
            log_sensor("조도(밝기)", LOG_WARNING, 0, "",
                       "햇빛이 부족합니다. 더 밝은 곳으로 옮겨주세요.",
                       0, 1);
            tts_talk("여기는 너무 어두워요. 제가 햇빛을 받을 수 있게, 창가로 옮겨주세요.");
        }
        else
        {
            strcpy(status_brightness, "");
            log_sensor("조도(밝기)", LOG_INFO, is_bright, "",
                       NULL, 0, 1);
        }

        if (config.fast_delay)
            delay(config.fast_delay);
        else
            delay_hour(1);
    }
}

void *t_LCD_Dot()
{
    char status[16];
    while (1)
    {
        strcpy(status, "");
        strcat(status, status_temperature);
        strcat(status, status_soilmoisture);
        strcat(status, status_brightness);

        if (strcmp(status, "") != 0) // 만약 식물의 온도, 토양습도, 조도 중 하나라도 문제가 있을 경우 status 배열 안에
        // 어떠한 값이 들어있을 것이므로 0이 아닐 것이다. 따라서, 이런 경우 bad condition으로
        {
            draw_sad();
            set_lcd_text(status);
        }

        else
        {
            draw_smile();
            set_lcd_text("Well Done!");
        }
    }
}

void *t_motion()
{
    int motion_detected = 0;
    while (1)
    {
        motion_detected = check_motion();
        if (motion_detected)
        {
            // 대화가 끝나면 60초 대기를 주는 파이썬 코드가 있으므로, 바로 대화가 시작될 걱정은 없다.
            send_localhost_text("MOTION_DETECTED", 50002, 1);
        }
    }
}

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);

    if (wiringPiSetup() == -1)
        return -1;

    dotmatrix_init();

    pthread_t threadId[5];
    int thread_status;
    int thread_join_status;

    if (config.enable_temperature)
    {
        thread_status = pthread_create(&threadId[0], NULL, t_temperature, NULL);
        if (thread_status < 0)
        {
            printf("온도센서 쓰레드 생성 오류\n");
            exit(0);
        }
    }

    if (config.enable_soilmoisture)
    {
        thread_status = pthread_create(&threadId[1], NULL, t_soilmoisture, NULL);
        if (thread_status < 0)
        {
            printf("토양습도센서 쓰레드 생성 오류\n");
            exit(0);
        }
    }

    if (config.enable_brightness)
    {
        thread_status = pthread_create(&threadId[2], NULL, t_brightness, NULL);
        if (thread_status < 0)
        {
            printf("조도센서 쓰레드 생성 오류\n");
            exit(0);
        }
    }

    thread_status = pthread_create(&threadId[3], NULL, t_LCD_Dot, NULL);
    if (thread_status < 0)
    {
        printf("LCD, 도트 매트릭스 쓰레드 생성 오류\n");
        exit(0);
    }

    if (config.enable_motionsensor)
    {
        thread_status = pthread_create(&threadId[4], NULL, t_motion, NULL);
        if (thread_status < 0)
        {
            printf("적외선 모션센서 쓰레드 생성 오류\n");
            exit(0);
        }
    }

    if (config.enable_temperature)
        pthread_join(threadId[0], (void **)&thread_join_status);
    if (config.enable_soilmoisture)
        pthread_join(threadId[1], (void **)&thread_join_status);
    if (config.enable_brightness)
        pthread_join(threadId[2], (void **)&thread_join_status);
    pthread_join(threadId[3], (void **)&thread_join_status);
    if (config.enable_motionsensor)
        pthread_join(threadId[4], (void **)&thread_join_status);

    return 0;
}