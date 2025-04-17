cmake_minimum_required(VERSION 3.16)

project(SlippiLiveDisplay VERSION 0.1 LANGUAGES CXX)

set(QT_NO_FIND_QMLSC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_package(Felgo REQUIRED)

set(PRODUCT_IDENTIFIER "at.cb.SlippiLiveDisplay")
set(PRODUCT_VERSION_NAME "1.1")
set(PRODUCT_VERSION_CODE 2)
set(PRODUCT_STAGE "publish")

# license for at.cb.SlippiLiveDisplay, version 2, plugins: none
set(PRODUCT_LICENSE_KEY "87E8E0E85909BC13E07B6881F5431F8A2EC4C648EEE94BF44D0E03AA486C2D30AF05DBD6FF7E475174938379341E812AA07822B36519A59BBB239C5B1BA0ACED2F77C680D526033A01CD7AA06CDFDCD392DCB8F6887127E67A841942EDEE1666892964F95BA53DB86623A31AAFC846FC359D1250289419F663CCFFE5AE7FF3B2D496282CEC73FAACF11BCFA33B7350A0E51BD12FC507A39F14E65E2B80FAE807EA06FB00A869D03BA1F284D703AD2D5692807E23902F1A41852988DDCB98C1E4D45416E0A58A9F61F6C4E230824DC33C2085C0CE1300C775571FD41C0724656C5D8EF1E522CB5CCF30082F3ACD73882781F59308E09CD54E547698B3EDDCB87C33EECDD032C50F340E9A9544D6A773A3A5262A4FB66A31315744C9A6B28C228A401582F11DA2ACDAA2ACABAECAD8AF17")

set(FELGO_PLUGINS)

file(GLOB_RECURSE QmlFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} qml/*.qml qml/*.js)

file(GLOB_RECURSE AssetsFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} assets/* qml/* qml/*/qmldir)
list(REMOVE_ITEM AssetsFiles ${QmlFiles})

file(GLOB_RECURSE SrcFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp src/*.h)

include_directories(src)
include_directories(include)

add_library(enet STATIC IMPORTED)
set_target_properties(enet PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/libs/enet64.lib")

qt_add_executable(SlippiLiveDisplay
    ${SrcFiles}
    ${QmlFiles}
    ${AssetsFiles}
)

felgo_configure_executable(SlippiLiveDisplay)

# Deploy resources to build folder/package directly
# comment for publishing
# deploy_resources("${QmlFiles};${AssetsFiles}")

# Add QML files and resources to QML module to included them via QRC automatically:
qt_add_qml_module(SlippiLiveDisplay
    URI SlippiLiveDisplay
    VERSION 1.0
# uncomment for publishing:
   QML_FILES ${QmlFiles}
   RESOURCES ${AssetsFiles}
    NO_RESOURCE_TARGET_PATH
)

target_compile_definitions(SlippiLiveDisplay
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(SlippiLiveDisplay PRIVATE Felgo
  enet ws2_32 winmm)

#find_package(FelgoLive REQUIRED)
#target_link_libraries(SlippiLiveDisplay PRIVATE FelgoLive)
