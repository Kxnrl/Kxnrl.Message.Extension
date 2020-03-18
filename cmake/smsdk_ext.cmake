if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(HL2SDK_64BIT ON)
    add_compile_options(-DPLATFORM_64BITS=1)
    message(STATUS "Building for 64 Bit")
else()
    set(HL2SDK_64BIT OFF)
    message(STATUS "Building for 32 Bit")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(OSX ON)
    add_compile_options(-DOSX -D_OSX)
    set(HL2SDK_LIB_STATIC_EXT ".a")
    set(HL2SDK_LIB_SHARED_EXT ".dylib")
    set(HL2SDK_LIB_PREFIX "lib")

    if(HL2SDK_64BIT)
        set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/osx64)
        set(HL2SDK_LIB_STATIC_SUFFIX "")
    else()
        set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/mac)
        set(HL2SDK_LIB_STATIC_SUFFIX "_i486")
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX ON)
    add_compile_options(-D_LINUX -DLINUX)
    set(HL2SDK_LIB_STATIC_EXT ".a")
    set(HL2SDK_LIB_SHARED_EXT ".so")
    set(HL2SDK_LIB_PREFIX "lib")

    if(HL2SDK_64BIT)
        set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/linux64)
        set(HL2SDK_LIB_STATIC_SUFFIX "")
    else()
        set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/linux)
        set(HL2SDK_LIB_STATIC_SUFFIX "_i486")
    endif()
elseif(WIN32)
    set(HL2SDK_LIB_DIR ${HL2SDK_PATH}/lib/public)
    set(HL2SDK_LIB_STATIC_EXT ".lib")
    set(HL2SDK_LIB_SHARED_EXT ".dll")
    set(HL2SDK_LIB_STATIC_SUFFIX "")
endif()

link_directories(${HL2SDK_LIB_DIR})

if(MSVC)
    add_compile_options(-DCOMPILER_MSVC)
    if(HL2SDK_64BIT)
        add_compile_options(-DCOMPILER_MSVC64)
     else()
        add_compile_options(-DCOMPILER_MSVC32)
     endif()
else()
    add_compile_options(-DCOMPILER_GCC) ## clang or gcc
endif()

# for mingw
if(MINGW)
    add_compile_options(
            -Dmalloc_usable_size=_msize
            -DUSE_STDC_FOR_SIMD=1
    )
    add_compile_options(-fpermissive)
endif()

if(NOT WIN32)
    set(POSIX ON)

    add_compile_options(
            -DVPROF_LEVEL=1 -DSWDS -D_finite=finite -Dstricmp=strcasecmp -D_stricmp=strcasecmp
            -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp
    )
endif()

if(POSIX)
    add_compile_options(-DPOSIX)
endif()

if(MSVC)
    add_compile_options(/wd4819 /wd4828 /wd5033 /permissive- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_compile_options(-Wno-register)
endif()

add_library(tier0 INTERFACE)
target_include_directories(tier0 INTERFACE
        ${HL2SDK_PATH}/public
        ${HL2SDK_PATH}/public/tier0
        ${CMAKE_SOURCE_DIR}/wrappers/msvc
        )
target_link_libraries(tier0 INTERFACE tier0${HL2SDK_LIB_STATIC_SUFFIX}${HL2SDK_LIB_STATIC_EXT})

add_library(tier1 STATIC
        ${HL2SDK_PATH}/tier1/bitbuf.cpp
        ${HL2SDK_PATH}/tier1/byteswap.cpp
        ${HL2SDK_PATH}/tier1/characterset.cpp
        ${HL2SDK_PATH}/tier1/checksum_crc.cpp
        ${HL2SDK_PATH}/tier1/checksum_md5.cpp
        ${HL2SDK_PATH}/tier1/commandbuffer.cpp
        ${HL2SDK_PATH}/tier1/convar.cpp
        ${HL2SDK_PATH}/tier1/datamanager.cpp
        ${HL2SDK_PATH}/tier1/diff.cpp
        ${HL2SDK_PATH}/tier1/generichash.cpp
        ${HL2SDK_PATH}/tier1/interface.cpp
        ${HL2SDK_PATH}/tier1/KeyValues.cpp
        ${HL2SDK_PATH}/tier1/mempool.cpp
        ${HL2SDK_PATH}/tier1/memstack.cpp
        ${HL2SDK_PATH}/tier1/NetAdr.cpp
        ${HL2SDK_PATH}/tier1/newbitbuf.cpp
        ${HL2SDK_PATH}/tier1/processor_detect.cpp
        ${HL2SDK_PATH}/tier1/rangecheckedvar.cpp
        ${HL2SDK_PATH}/tier1/stringpool.cpp
        ${HL2SDK_PATH}/tier1/strtools.cpp
        ${HL2SDK_PATH}/tier1/tier1.cpp
        ${HL2SDK_PATH}/tier1/undiff.cpp
        ${HL2SDK_PATH}/tier1/uniqueid.cpp
        ${HL2SDK_PATH}/tier1/utlbuffer.cpp
        ${HL2SDK_PATH}/tier1/utlbufferutil.cpp
        ${HL2SDK_PATH}/tier1/utlstring.cpp
        ${HL2SDK_PATH}/tier1/utlsymbol.cpp
        )
target_link_libraries(tier1 tier0)
target_include_directories(tier1 PUBLIC
        ${HL2SDK_PATH}/public
        ${HL2SDK_PATH}/public/tier1
        )

add_library(mathlib STATIC
        ${HL2SDK_PATH}/mathlib/anorms.cpp
        ${HL2SDK_PATH}/mathlib/bumpvects.cpp
        ${HL2SDK_PATH}/mathlib/color_conversion.cpp
        ${HL2SDK_PATH}/mathlib/halton.cpp
        ${HL2SDK_PATH}/mathlib/IceKey.cpp
        ${HL2SDK_PATH}/mathlib/imagequant.cpp
        ${HL2SDK_PATH}/mathlib/lightdesc.cpp
        ${HL2SDK_PATH}/mathlib/mathlib_base.cpp
        ${HL2SDK_PATH}/mathlib/polyhedron.cpp
        ${HL2SDK_PATH}/mathlib/powsse.cpp
        ${HL2SDK_PATH}/mathlib/quantize.cpp
        ${HL2SDK_PATH}/mathlib/randsse.cpp
        ${HL2SDK_PATH}/mathlib/simdvectormatrix.cpp
        ${HL2SDK_PATH}/mathlib/sparse_convolution_noise.cpp
        ${HL2SDK_PATH}/mathlib/sse.cpp
        ${HL2SDK_PATH}/mathlib/sseconst.cpp
        ${HL2SDK_PATH}/mathlib/ssenoise.cpp
        ${HL2SDK_PATH}/mathlib/vector.cpp
        ${HL2SDK_PATH}/mathlib/vmatrix.cpp
        )
if(NOT MSVC)
    target_compile_options(mathlib PRIVATE -Wno-c++11-narrowing)
endif()
target_link_libraries(mathlib PUBLIC tier0 tier1)
target_include_directories(mathlib PUBLIC ${HL2SDK_PATH}/public/mathlib)

add_library(vstdlib INTERFACE)
target_include_directories(vstdlib INTERFACE
        ${HL2SDK_PATH}/public
        ${HL2SDK_PATH}/public/vstdlib
        )
target_link_libraries(vstdlib INTERFACE vstdlib${HL2SDK_LIB_STATIC_SUFFIX}${HL2SDK_LIB_STATIC_EXT})

add_library(interfaces STATIC
        ${HL2SDK_PATH}/interfaces/interfaces.cpp
        )
target_include_directories(interfaces PUBLIC
        ${HL2SDK_PATH}/public
        ${HL2SDK_PATH}/public/interfaces
        )

# brew install protobuf
# vcpkg install protobuf
#find_package(Protobuf REQUIRED)
#include_directories(${Protobuf_INCLUDE_DIRS})
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
# hack for msvc who can't find google/protobuf/*
#set(Protobuf_IMPORT_DIRS ${Protobuf_IMPORT_DIRS} ${Protobuf_INCLUDE_DIRS})
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
#        "${HL2SDK_PATH}/public/engine/protobuf/netmessages.proto"
#        "${HL2SDK_PATH}/public/game/shared/csgo/protobuf/cstrike15_usermessages.proto"
#        )
set(PROTO_SRCS ${HL2SDK_PATH}/public/engine/protobuf/netmessages.pb.cc ${HL2SDK_PATH}/public/game/shared/csgo/protobuf/cstrike15_usermessages.pb.cc)
set(PROTO_HDRS ${HL2SDK_PATH}/public/engine/protobuf/netmessages.pb.h ${HL2SDK_PATH}/public/game/shared/csgo/protobuf/cstrike15_usermessages.pb.h)

add_library(hlsdk_protobuf STATIC 
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc         
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc        
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/stubs/common.cc                              
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/stubs/once.cc                                
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/stubs/stringprintf.cc                        
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/extension_set.cc                             
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/generated_message_util.cc                    
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/message_lite.cc                              
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/repeated_field.cc                            
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/wire_format_lite.cc                          
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/io/coded_stream.cc                           
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/io/zero_copy_stream.cc                       
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/io/zero_copy_stream_impl_lite.cc
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/stubs/strutil.cc                             
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/stubs/substitute.cc                          
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/stubs/structurally_valid.cc                  
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/descriptor.cc                                
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/descriptor.pb.cc                             
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/descriptor_database.cc                       
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/dynamic_message.cc                           
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/extension_set_heavy.cc                       
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/generated_message_reflection.cc              
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/message.cc                                   
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/reflection_ops.cc                            
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/service.cc                                   
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/text_format.cc                               
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/unknown_field_set.cc                         
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/wire_format.cc                               
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/io/gzip_stream.cc                            
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/io/printer.cc                                
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/io/tokenizer.cc                              
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/io/zero_copy_stream_impl.cc                  
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/compiler/importer.cc                         
	${HL2SDK_PATH}/common/protobuf-2.5.0/src/google/protobuf/compiler/parser.cc
)
target_include_directories(hlsdk_protobuf PUBLIC 
	${HL2SDK_PATH}/common/protobuf-2.5.0/src
	)
target_include_directories(hlsdk_protobuf PRIVATE 
	${CMAKE_SOURCE_DIR}/wrappers/protobuf
	)

add_library(hlsdk_proto STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(hlsdk_proto PUBLIC 
	${HL2SDK_PATH}/public/engine/protobuf
	${HL2SDK_PATH}/public/game/shared/csgo/protobuf
	)
target_link_libraries(hlsdk_proto PUBLIC hlsdk_protobuf)

add_library(amtl INTERFACE)
target_include_directories(amtl INTERFACE
        ${SOURCEMOD_PATH}/public/amtl
        ${SOURCEMOD_PATH}/public/amtl/amtl
        )

add_library(smsdk INTERFACE)
target_include_directories(smsdk INTERFACE
        ${SOURCEMOD_PATH}/core
        ${SOURCEMOD_PATH}/public
        ${SOURCEMOD_PATH}/sourcepawn/include
        )
target_link_libraries(smsdk INTERFACE amtl)

add_library(mmsdk INTERFACE)
target_include_directories(mmsdk INTERFACE
        ${METAMOD_SOURCE_PATH}/core
        ${METAMOD_SOURCE_PATH}/core/sourcehook
        )

add_library(smsdk_ext INTERFACE)
target_sources(smsdk_ext INTERFACE ${SOURCEMOD_PATH}/public/smsdk_ext.cpp)
target_compile_definitions(smsdk_ext INTERFACE
        -DSE_EPISODEONE=1 -DSE_DARKMESSIAH=2 -DSE_ORANGEBOX=3 -DSE_BLOODYGOODTIME=4 -DSE_EYE=5 -DSE_CSS=6 -DSE_ORANGEBOXVALVE=7 -DSE_LEFT4DEAD=8 -DSE_LEFT4DEAD2=9 -DSE_ALIENSWARM=10 -DSE_PORTAL2=11 -DSE_CSGO=12
        )
target_compile_definitions(smsdk_ext INTERFACE
        -DSDK_EXPORTS -DSOURCEMOD_BUILD -DSOURCE_ENGINE=12)
target_link_libraries(smsdk_ext INTERFACE smsdk tier0 tier1 mathlib vstdlib interfaces mmsdk)
