#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include "fingerprint_api.h"

void fingerprint_callback(int status, int template_id, void* user_data) {
    printf("Callback: status=%d, template_id=%d\n", status, template_id);
    
    if (status == FP_STATUS_FINGER_DETECTED) {
        printf("检测到已注册的指纹，ID: %d\n", template_id);
    }
    else if (status == FP_ENROLL_PLACE_FINGER) {
        printf("请放置指纹 (步骤 %d/3)\n", template_id + 1);
    }
    else if (status == FP_ENROLL_REMOVE_FINGER) {
        printf("请移开指纹 (步骤 %d/3)\n", template_id + 1);
    }
    else if (status == FP_ENROLL_MERGE) {
        printf("指纹模板创建成功，可以保存\n");
    }
    else if (status == FP_ENROLL_DUPLICATE) {
        printf("指纹已存在，请使用其他手指\n");
    }
}

int main(int argc, char** argv) {
    const char* device_path = "/dev/sg0"; // 默认设备路径
    
    // 如果提供了命令行参数，使用它作为设备路径
    if (argc > 1) {
        device_path = argv[1];
    }
    
    printf("初始化指纹库...\n");
    printf("设备路径: %s\n", device_path);
    
    int ret = fp_init(device_path);
    if (ret != FP_SUCCESS) {
        printf("初始化指纹库失败: %d\n", ret);
        return 1;
    }
    
    printf("指纹库初始化成功\n");
    printf("当前可用ID: %d\n", fp_get_available_id());
    
    char choice;
    printf("选择操作模式:\n");
    printf("1. 指纹检测模式\n");
    printf("2. 指纹注册模式\n");
    printf("3. 删除指纹模板\n");
    printf("请输入选择 (1-3): ");
    scanf(" %c", &choice);
    
    switch (choice) {
        case '1':
            printf("启动指纹检测...\n");
            fp_start_detection(fingerprint_callback, NULL);
            
            printf("检测模式将运行30秒，请在此期间尝试放置已注册的指纹...\n");
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
            printf("停止指纹检测...\n");
            fp_stop_detection();
            break;
            
        case '2':
            printf("启动指纹注册...\n");
            fp_start_enrollment(fingerprint_callback, NULL);
            
            printf("请按照提示完成指纹注册过程...\n");
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
            // 询问是否保存模板
            printf("是否保存指纹模板? (y/n): ");
            scanf(" %c", &choice);
            if (choice == 'y' || choice == 'Y') {
                int id = fp_get_available_id();
                if (id > 0) {
                    printf("保存指纹模板，ID: %d\n", id);
                    fp_save_template(id);
                } else {
                    printf("无法获取可用ID，请尝试删除一些模板后再试\n");
                }
            }
            break;
            
        case '3':
            {
                int id;
                printf("请输入要删除的指纹ID: ");
                scanf("%d", &id);
                
                printf("删除指纹ID %d...\n", id);
                ret = fp_delete_template(id);
                if (ret == FP_SUCCESS) {
                    printf("删除成功\n");
                } else {
                    printf("删除失败: %d\n", ret);
                }
            }
            break;
            
        default:
            printf("无效选择\n");
            break;
    }
    
    printf("清理资源...\n");
    fp_cleanup();
    
    printf("测试完成\n");
    return 0;
}