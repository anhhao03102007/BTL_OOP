cmake_minimum_required(VERSION 3.16)
project(BankingSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Đường dẫn Qt6
#    Chỉ dùng đường dẫn cứng khi bạn CHƯA truyền Qt từ bên ngoài.
#    - Trên GitHub Actions, install-qt-action tự đặt QT_ROOT_DIR nên đoạn này bị bỏ qua.
#    - Trên máy bạn, vẫn dùng đường dẫn dưới đây (hoặc truyền -DCMAKE_PREFIX_PATH=...).
if(NOT CMAKE_PREFIX_PATH AND NOT DEFINED ENV{QT_ROOT_DIR} AND NOT DEFINED ENV{Qt6_DIR})
    set(CMAKE_PREFIX_PATH "C:/Qt/6.12.0/mingw_64")
endif()

# 2. Tự động xử lý MOC, UIC, RCC cho Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 3. Tìm các module Qt6 cần thiết
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# 4. Thư mục include để dùng #include "service/UserService.hpp"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 5. Thu gom file nguồn (CONFIGURE_DEPENDS: thêm file .cpp mới thì CMake tự nhận)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "main.cpp"
)

# 6. Thu gom header để AUTOMOC theo dõi
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "include/*.hpp"
)

# 7. Tạo file thực thi
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 8. Liên kết thư viện Qt6
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# 9. (Tùy chọn) Ẩn cửa sổ console đen khi chạy app. Bỏ dấu # nếu muốn dùng.
#    Lưu ý: bật lên thì sẽ không thấy qDebug() trong console.
# set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)