LOCAL_PATH := $(call my-dir)
LOCAL_SRC_FILES := medialibrary.cpp AndroidMediaLibrary.cpp AndroidDeviceLister.cpp utils.cpp
LOCAL_MODULE    := mla
LOCAL_MODULE_FILENAME := libmla
LOCAL_LDLIBS    += $(MEDIALIBRARY_LIBS) $(VLC_LIBS) $(LIBJPEG_LIBS) $(SQLITE_LIBS) -llog
LOCAL_C_INCLUDES := $(MEDIALIBRARY_INCLUDE_DIR)
include $(BUILD_SHARED_LIBRARY)
