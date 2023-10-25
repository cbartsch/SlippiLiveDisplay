cmake_minimum_required(VERSION 3.16)

project(SlippiLiveDisplay VERSION 0.1 LANGUAGES CXX)

set(QT_NO_FIND_QMLSC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_package(Felgo REQUIRED)

set(PRODUCT_IDENTIFIER "com.yourcompany.wizardEVAP.SlippiLiveDisplay")
set(PRODUCT_VERSION_NAME "1.0.0")
set(PRODUCT_VERSION_CODE 1)
set(PRODUCT_STAGE "test")
set(PRODUCT_LICENSE_KEY "")

set(FELGO_PLUGINS)

file(GLOB_RECURSE QmlFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml qml/*.js)

file(GLOB_RECURSE AssetsFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} assets/* qml/*)
list(REMOVE_ITEM AssetsFiles ${QmlFiles})

file(GLOB_RECURSE SrcFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp src/*.h)

include_directories(src)
include_directories(include)

add_library(enet STATIC IMPORTED)
set_target_properties(enet PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/libs/enet64.lib")

qt_add_executable(appSlippiLiveDisplay
    main ${SrcFiles}
    ${QmlFiles}
    ${AssetsFiles}
)

felgo_configure_executable(appSlippiLiveDisplay)

# Deploy resources to build folder/package directly
# comment for publishing
deploy_resources("${QmlFiles};${AssetsFiles}")

# Add QML files and resources to QML module to included them via QRC automatically:
qt_add_qml_module(appSlippiLiveDisplay
    URI SlippiLiveDisplay
    VERSION 1.0
# uncomment for publishing:
#    QML_FILES ${QmlFiles}
#    RESOURCES ${AssetsFiles}
    NO_RESOURCE_TARGET_PATH
)

target_compile_definitions(appSlippiLiveDisplay
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(appSlippiLiveDisplay PRIVATE Felgo
  enet ws2_32 winmm)

#find_package(FelgoLive REQUIRED)
#target_link_libraries(appSlippiLiveDisplay PRIVATE FelgoLive)
