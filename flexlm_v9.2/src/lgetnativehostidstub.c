/*
 * This is a stub of the Java hostid interface routine, intended
 * for platforms on which we don't directly support Java.
 * It is here so that a single .def file which defines the DLL contents
 * handles both Java-supported and non-supported platforms.  On supported
 * platforms the object module created from this file is replaced by the
 * real one.
 */

int Java_com_macrovision_flexlm_HostId_lGetNativeHostId() {
	return 0;
}
