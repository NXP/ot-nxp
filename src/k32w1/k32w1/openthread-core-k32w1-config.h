/*
 *  Copyright (c) 2017, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes k32w1 compile-time configuration constants
 *   for OpenThread.
 */

#ifndef OPENTHREAD_CORE_K32W1_CONFIG_H_
#define OPENTHREAD_CORE_K32W1_CONFIG_H_

/**
 * @def OPENTHREAD_CONFIG_PLATFORM_INFO
 *
 * The platform-specific string to insert into the OpenThread version string.
 *
 */
#define OPENTHREAD_CONFIG_PLATFORM_INFO "K32W1"

/**
 * @def OPENTHREAD_CONFIG_LOG_OUTPUT
 *
 * The k32w1 platform provides an otPlatLog() function.
 */
#ifndef OPENTHREAD_CONFIG_LOG_OUTPUT /* allow command line override */
#define OPENTHREAD_CONFIG_LOG_OUTPUT OPENTHREAD_CONFIG_LOG_OUTPUT_NONE
#endif

/**
 * @def OPENTHREAD_CONFIG_LOG_LEVEL
 *
 * The log level (used at compile time). If `OPENTHREAD_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL`
 * is set, this defines the most verbose log level possible. See
 *`OPENTHREAD_CONFIG_INITIAL_LOG_LEVEL` to set the initial log level.
 *
 */
#ifndef OPENTHREAD_CONFIG_LOG_LEVEL
#define OPENTHREAD_CONFIG_LOG_LEVEL OT_LOG_LEVEL_NONE
#endif

/**
 * @def OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE
 *
 * Define to 1 if you want to enable microsecond backoff timer implemented in platform.
 *
 */
#ifndef OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE
#define OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_PLATFORM_FLASH_API_ENABLE
 *
 * Define to 1 to enable otPlatFlash* APIs to support non-volatile storage.
 *
 * When defined to 1, the platform MUST implement the otPlatFlash* APIs instead of the otPlatSettings* APIs.
 *
 */
#ifndef OPENTHREAD_CONFIG_PLATFORM_FLASH_API_ENABLE
#define OPENTHREAD_CONFIG_PLATFORM_FLASH_API_ENABLE 0
#endif

/**
 * @def OPENTHREAD_CONFIG_JOINER_ENABLE
 *
 * Define to 1 to enable Joiner support.
 *
 */
#ifndef OPENTHREAD_CONFIG_JOINER_ENABLE
#define OPENTHREAD_CONFIG_JOINER_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_COMMISSIONER_ENABLE
 *
 * Define to 1 to enable Commissioner support.
 *
 */
#ifndef OPENTHREAD_CONFIG_COMMISSIONER_ENABLE
#define OPENTHREAD_CONFIG_COMMISSIONER_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_HEAP_INTERNAL_SIZE
 *
 * The size of heap buffer when DTLS is enabled.
 *
 */
#ifndef OPENTHREAD_CONFIG_HEAP_INTERNAL_SIZE
#define OPENTHREAD_CONFIG_HEAP_INTERNAL_SIZE (2048 * sizeof(void *))
#endif

/**
 * @def OPENTHREAD_CONFIG_UDP_FORWARD_ENABLE
 *
 * Define to 1 to enable UDP forward support.
 *
 */
#ifndef OPENTHREAD_CONFIG_UDP_FORWARD_ENABLE
#define OPENTHREAD_CONFIG_UDP_FORWARD_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE
 *
 * Define to 1 to enable the Border Router service.
 *
 */
#ifndef OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE
#define OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_DHCP6_CLIENT_ENABLE
 *
 * Define to 1 to enable the DHCP CLIENT service.
 *
 */
#ifndef OPENTHREAD_CONFIG_DHCP6_CLIENT_ENABLE
#define OPENTHREAD_CONFIG_DHCP6_CLIENT_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE
 *
 * Define to 1 to enable the DHCP SERVER service.
 *
 */
#ifndef OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE
#define OPENTHREAD_CONFIG_DHCP6_SERVER_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE
 *
 * Define as 1 to enable Link Metrics initiator feature.
 *
 */
#ifndef OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE
#define OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
 *
 * Define as 1 to enable Link Metrics subject feature.
 *
 */
#ifndef OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
#define OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_MAC_SOFTWARE_RETRANSMIT_ENABLE
 *
 * Define to 1 if you want to enable software retransmission logic.
 *
 */
#ifndef OPENTHREAD_CONFIG_MAC_SOFTWARE_RETRANSMIT_ENABLE
#define OPENTHREAD_CONFIG_MAC_SOFTWARE_RETRANSMIT_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_MAC_SOFTWARE_CSMA_BACKOFF_ENABLE
 *
 * Define to 1 if you want to enable software CSMA-CA backoff logic.
 *
 */
#ifndef OPENTHREAD_CONFIG_MAC_SOFTWARE_CSMA_BACKOFF_ENABLE
#define OPENTHREAD_CONFIG_MAC_SOFTWARE_CSMA_BACKOFF_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_NCP_HDLC_ENABLE
 *
 * Define to 1 to enable NCP HDLC support.
 *
 */
#ifndef OPENTHREAD_CONFIG_NCP_HDLC_ENABLE
#define OPENTHREAD_CONFIG_NCP_HDLC_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_NCP_TX_BUFFER_SIZE
 *
 * The size of NCP message buffer in bytes.
 *
 */
#ifndef OPENTHREAD_CONFIG_NCP_TX_BUFFER_SIZE
#define OPENTHREAD_CONFIG_NCP_TX_BUFFER_SIZE 4096
#endif

/**
 * @def OPENTHREAD_CONFIG_MLE_MAX_CHILDREN
 *
 * The maximum number of children.
 *
 */
#ifndef OPENTHREAD_CONFIG_MLE_MAX_CHILDREN
#define OPENTHREAD_CONFIG_MLE_MAX_CHILDREN 30
#endif

/**
 * @def OPENTHREAD_CONFIG_DUA_ENABLE
 *
 * Define as 1 to support Thread 1.2 Domain Unicast Address feature.
 *
 */
#ifndef OPENTHREAD_CONFIG_DUA_ENABLE
#define OPENTHREAD_CONFIG_DUA_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_MLR_ENABLE
 *
 * Define as 1 to support Thread 1.2 Multicast Listener Registration feature.
 *
 */
#ifndef OPENTHREAD_CONFIG_MLR_ENABLE
#define OPENTHREAD_CONFIG_MLR_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
 *
 * This setting configures the CSL receiver feature in Thread 1.2.
 *
 */
#ifndef OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
#define OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE 1
#endif

/**
 * @def OPENTHREAD_CONFIG_MAC_CSL_AUTO_SYNC_ENABLE
 *
 * This setting configures CSL auto synchronization based on data poll mechanism in Thread 1.2.
 *
 */
#undef OPENTHREAD_CONFIG_MAC_CSL_AUTO_SYNC_ENABLE
#define OPENTHREAD_CONFIG_MAC_CSL_AUTO_SYNC_ENABLE OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE

/**
 * @def OPENTHREAD_CONFIG_MAC_SOFTWARE_TX_TIMING_ENABLE
 *
 * Define to 1 to enable software transmission target time logic.
 *
 */
#ifndef OPENTHREAD_CONFIG_MAC_SOFTWARE_TX_TIMING_ENABLE
#define OPENTHREAD_CONFIG_MAC_SOFTWARE_TX_TIMING_ENABLE 1
#endif

/* The  accuracy, in units of +/- ppm, of the clock used for scheduling CSL operations */
#ifndef CONFIG_PLATFORM_CSL_ACCURACY
#define CONFIG_PLATFORM_CSL_ACCURACY 40
#endif

/* Should cover Tx tune time (warm-up) + encryption time +
   us timer inaccuracy (it uses ticks ~= 30.5us) + RCP communication overhead */
/**
 * @def OPENTHREAD_CONFIG_MAC_CSL_REQUEST_AHEAD_US
 *
 * Define how many microseconds ahead should MAC deliver CSL frame to SubMac.
 *
 */
#ifndef OPENTHREAD_CONFIG_MAC_CSL_REQUEST_AHEAD_US
#define OPENTHREAD_CONFIG_MAC_CSL_REQUEST_AHEAD_US 3500
#endif

/**
 * @def OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS
 *
 * The number of message buffers in the buffer pool.
 *
 */
#ifndef OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS
#define OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS 160
#endif

#endif // OPENTHREAD_CORE_K32W1_CONFIG_H_
