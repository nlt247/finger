#include <iostream>
#include "fingerprint_api.h"

int main(int argc, char* argv[]) {
    const char* device = "/dev/sr0";  // 默认设备路径

    if (argc > 1) {
        device = argv[1];
    }

    std::cout << "正在初始化指纹设备: " << device << std::endl;

    int ret = fp_init_connection(device);
    if (ret != true) {
        std::cerr << "初始化失败！" << std::endl;
        return 1;
    }

    std::cout << "初始化成功！请将手指放在传感器上..." << std::endl;

    int detect_result = fp_finger_detect();
    if (detect_result <= 0) {
        std::cerr << "未检测到手指或检测失败。" << std::endl;
        fp_close_connection();
        return 1;
    }

    std::cout << "检测到手指！正在获取图像..." << std::endl;
    ret = fp_get_image();
    if (ret != true) {
        std::cerr << "获取图像失败。" << std::endl;
        fp_close_connection();
        return 1;
    }

    std::cout << "图像获取成功！正在生成特征..." << std::endl;
    ret = fp_generate(1);  // FP_BUFFER_1 = 1
    if (ret != true) {
        std::cerr << "特征生成失败。" << std::endl;
        fp_close_connection();
        return 1;
    }

    int tmpl_no = 0;
    std::cout << "正在搜索指纹..." << std::endl;
    ret = fp_search(&tmpl_no);
    if (ret) {
        std::cout << "找到匹配的指纹，ID: " << tmpl_no << std::endl;
    } else {
        std::cout << "未找到匹配指纹，准备存储..." << std::endl;

        ret = fp_get_empty_id(1);
        if (ret != true) {
            std::cerr << "查找空位失败。" << std::endl;
            fp_close_connection();
            return 1;
        }

        int empty_id = 0;
        if (fp_get_empty_id(1)) {
            std::cout << "找到空ID，正在存储指纹..." << std::endl;
            ret = fp_store_char(empty_id);
            if (ret != true) {
                std::cerr << "存储失败。" << std::endl;
            } else {
                std::cout << "指纹成功存储为ID: " << empty_id << std::endl;
            }
        } else {
            std::cerr << "未能获取空ID" << std::endl;
        }
    }

    fp_close_connection();
    std::cout << "连接已关闭" << std::endl;

    return 0;
}
