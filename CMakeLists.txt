cmake_minimum_required(VERSION 3.16)
project(QtDemo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trỏ chính xác đến thư mục phiên bản Qt 6.12.0 của bạn (nhớ đổi ổ đĩa nếu bạn cài ở D: hoặc E:)
set(CMAKE_PREFIX_PATH "C:/Qt/6.12.0/mingw_64") 

# Tự động xử lý các tính năng của Qt (MOC, UIC, RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Tìm các module cần thiết của Qt6 (Core, Widgets,...)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Tạo file thực thi từ mã nguồn main.cpp
add_executable(QtDemo main.cpp)

# Liên kết các thư viện Qt vào chương trình
target_link_libraries(QtDemo PRIVATE Qt6::Core Qt6::Widgets)