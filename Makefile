#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := garbage_classification
EXTRA_COMPONENT_DIRS += ../../components/

include $(IDF_PATH)/make/project.mk
