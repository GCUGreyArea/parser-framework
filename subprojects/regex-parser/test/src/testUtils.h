#ifndef _TEST_UTILS_
#define _TEST_UTILS_

/**
 * @file testUtils.h
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief test header for utilities for the filescan application
 * @version 0.1
 * @date 2021-08-31
 * 
 * @copyright Copyright (c) 2021
 * @addtogroup test
 * @{
 * @addtogroup utils
 * @{
 */

void randomiseString(char * str);
void buildRandomTestFile(const char * fillPathAndName, const char * testFrase, int size);
void buildNormalTestFile(const char * fillPathAndName, const char * testFrase, size_t buffSize, int writeTimes, bool addIdx);

/**
 * @}
 * @}
 */

#endif//_TEST_UTILS_