# Settings for stm32f4 based platforms
ifeq ($(TARGET), stm32f4)
  export PATH := $(MAKEFILE_DIR)/downloads/gcc_embedded/bin/:$(PATH)
  TARGET_ARCH := cortex-m4
  TARGET_TOOLCHAIN_PREFIX := arm-none-eabi-
  TARGET_TOOLCHAIN_ROOT := $(TENSORFLOW_ROOT)$(MAKEFILE_DIR)/downloads/gcc_embedded/bin/

  $(eval $(call add_third_party_download,$(GCC_EMBEDDED_URL),$(GCC_EMBEDDED_MD5),gcc_embedded,))
  $(eval $(call add_third_party_download,$(CMSIS_URL),$(CMSIS_MD5),cmsis,patch_cmsis))
  $(eval $(call add_third_party_download,$(STM32_BARE_LIB_URL),$(STM32_BARE_LIB_MD5),stm32_bare_lib,))

  # TODO(b/161478030) : change - Wno - vla to - Wvla and remove - Wno-shadow once
  # we have a solution for fixing / avoiding being tripped up by these warnings.
  PLATFORM_FLAGS = \
    -DGEMMLOWP_ALLOW_SLOW_SCALAR_FALLBACK \
    -DTF_LITE_STATIC_MEMORY \
    -DTF_LITE_MCU_DEBUG_LOG \
    -fmessage-length=0 \
    -fno-exceptions \
    -fno-unwind-tables \
    -ffunction-sections \
    -fdata-sections \
    -funsigned-char \
    -MMD \
    -mcpu=cortex-m4 \
    -mthumb \
    -Wall \
    -Wextra \
    -Wno-shadow \
    -Wno-vla \
    -Wno-strict-aliasing \
    -Wno-type-limits \
    -Wno-unused-parameter \
    -Wno-missing-field-initializers \
    -Wno-write-strings \
    -Wno-sign-compare \
    -Wunused-function \
    -fno-delete-null-pointer-checks \
    -fomit-frame-pointer \
    -g \
    -Os
  CXXFLAGS += $(PLATFORM_FLAGS) -std=gnu++11 -fno-rtti -fno-use-cxa-atexit
  CCFLAGS += $(PLATFORM_FLAGS)
  LDFLAGS += \
    --specs=nosys.specs \
    -T ${TENSORFLOW_ROOT}$(MAKEFILE_DIR)/targets/stm32f4/stm32f4.lds \
    -Wl,-Map=${TENSORFLOW_ROOT}$(MAKEFILE_DIR)/gen/$(TARGET).map,--cref \
    -Wl,--gc-sections
  BUILD_TYPE := micro
  MICROLITE_LIBS := \
    -lm
  INCLUDES += \
    -isystem$(MAKEFILE_DIR)/downloads/cmsis/CMSIS/Core/Include/ \
    -I$(MAKEFILE_DIR)/downloads/stm32_bare_lib/include/
  THIRD_PARTY_CC_SRCS += \
    $(wildcard $(MAKEFILE_DIR)/downloads/stm32_bare_lib/source/*.c) \
    $(wildcard $(MAKEFILE_DIR)/downloads/stm32_bare_lib/source/*.cc)
  EXCLUDED_SRCS := \
    $(MAKEFILE_DIR)/downloads/stm32_bare_lib/source/debug_log.c
  THIRD_PARTY_CC_SRCS  := $(filter-out $(EXCLUDED_SRCS), $(THIRD_PARTY_CC_SRCS))
  MICROLITE_CC_SRCS := $(filter-out $(EXCLUDED_SRCS), $(MICROLITE_CC_SRCS))
  TEST_SCRIPT := tensorflow/lite/micro/testing/test_stm32f4_binary.sh
  # TODO, non working tests.. the micro_speech example partly works
  # TODO(b/158324045): Examine why some tests fail here.

  EXCLUDED_TESTS := \
    tensorflow/lite/micro/micro_interpreter_test.cc \
    tensorflow/lite/micro/micro_allocator_test.cc \
    tensorflow/lite/micro/memory_helpers_test.cc \
    tensorflow/lite/micro/memory_arena_threshold_test.cc \
    tensorflow/lite/micro/recording_micro_allocator_test.cc \
    tensorflow/lite/micro/kernels/circular_buffer_test.cc \
    tensorflow/lite/micro/kernels/conv_test.cc \
    tensorflow/lite/micro/kernels/fully_connected_test.cc \
    tensorflow/lite/micro/kernels/detection_postprocess_test.cc
  MICROLITE_TEST_SRCS := $(filter-out $(EXCLUDED_TESTS), $(MICROLITE_TEST_SRCS))

  EXCLUDED_EXAMPLE_TESTS := \
    tensorflow/lite/micro/examples/magic_wand/Makefile.inc \
    tensorflow/lite/micro/examples/person_detection/Makefile.inc \
    tensorflow/lite/micro/examples/person_detection_experimental/Makefile.inc \
    tensorflow/lite/micro/examples/micro_speech/Makefile.inc \
    tensorflow/lite/micro/examples/image_recognition_experimental/Makefile.inc
  MICRO_LITE_EXAMPLE_TESTS := $(filter-out $(EXCLUDED_EXAMPLE_TESTS), $(MICRO_LITE_EXAMPLE_TESTS))

# These are microcontroller-specific rules for converting the ELF output
# of the linker into a binary image that can be loaded directly.
OBJCOPY := $(TARGET_TOOLCHAIN_PREFIX)objcopy

$(BINDIR)/%.bin: $(BINDIR)/%
	@mkdir -p $(dir $@)
	$(OBJCOPY) $< $@ -O binary

endif
