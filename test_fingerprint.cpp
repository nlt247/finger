#include <iostream>
#include "fingerprint_api.h"

int main(int argc, char* argv[]) {
    const char* device = "/dev/ttyUSB0";  // 默认设备
    
    if (argc > 1) {
        device = argv[1];  // 使用命令行参数指定设备
    }
    
    std::cout << "正在初始化指纹设备: " << device << std::endl;
    
    int ret = fp_init_connection(device);
    if (ret != FP_SUCCESS) {
        std::cerr << "初始化失败，错误码: " << ret << std::endl;
        return 1;
    }
    
    std::cout << "初始化成功!" << std::endl;
    
    // 测试检测手指
    int detect_result = 0;
    std::cout << "请将手指放在传感器上..." << std::endl;
    ret = fp_finger_detect(&detect_result);
    
    if (ret != FP_SUCCESS) {
        std::cerr << "检测失败，错误码: " << ret << std::endl;
    } else {
        if (detect_result) {
            std::cout << "检测到手指!" << std::endl;
            
            // 获取图像
            std::cout << "正在获取指纹图像..." << std::endl;
            ret = fp_get_image();
            
            if (ret != FP_SUCCESS) {
                std::cerr << "获取图像失败，错误码: " << ret << std::endl;
            } else {
                std::cout << "图像获取成功!" << std::endl;
                
                // 生成特征
                std::cout << "正在生成特征..." << std::endl;
                ret = fp_generate(FP_BUFFER_1);
                
                if (ret != FP_SUCCESS) {
                    std::cerr << "生成特征失败，错误码: " << ret << std::endl;
                } else {
                    std::cout << "特征生成成功!" << std::endl;
                    
                    // 搜索指纹
                    int tmpl_no = 0;
                    int learn_result = 0;
                    
                    std::cout << "正在搜索指纹..." << std::endl;
                    ret = fp_search(FP_BUFFER_1, 0, 1000, &tmpl_no, &learn_result);
                    
                    if (ret != FP_SUCCESS) {
                        std::cerr << "搜索失败，错误码: " << ret << std::endl;
                    } else {
                        if (tmpl_no > 0) {
                            std::cout << "找到匹配的指纹，ID: " << tmpl_no << std::endl;
                        } else {
                            std::cout << "未找到匹配的指纹" << std::endl;
                            
                            // 存储新指纹
                            std::cout << "正在查找空ID..." << std::endl;
                            int empty_id = 0;
                            ret = fp_get_empty_id(1, 1000, &empty_id);
                            
                            if (ret != FP_SUCCESS) {
                                std::cerr << "查找空ID失败，错误码: " << ret << std::endl;
                            } else {
                                std::cout << "找到空ID: " << empty_id << std::endl;
                                
                                // 存储指纹
                                int dup_tmpl_no = 0;
                                std::cout << "正在存储指纹..." << std::endl;
                                ret = fp_store_char(empty_id, FP_BUFFER_1, &dup_tmpl_no);
                                
                                if (ret != FP_SUCCESS) {
                                    std::cerr << "存储指纹失败，错误码: " << ret << std::endl;
                                } else {
                                    if (dup_tmpl_no > 0) {
                                        std::cout << "发现重复指纹，ID: " << dup_tmpl_no << std::endl;
                                    } else {
                                        std::cout << "指纹成功存储为ID: " << empty_id << std::endl;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            std::cout << "未检测到手指" << std::endl;
        }
    }
    
    // 关闭连接
    fp_close_connection();
    std::cout << "连接已关闭" << std::endl;
    
    return 0;
}