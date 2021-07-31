#ifndef MAILBOX_H
#define MAILBOX_H

#include "kernel/kernel.h"
#include "bcm2836.h"


 // Tag Framebuffer
#define BCM2835_VC_TAG_ALLOCATE_BUFFER			0x00040001	///<
#define BCM2835_VC_TAG_RELEASE_BUFFER			0x00048001	///<
#define BCM2835_VC_TAG_BLANK_SCREEN				0x00040002	///<
#define BCM2835_VC_TAG_GET_PHYS_WH				0x00040003	///<
#define BCM2835_VC_TAG_TEST_PHYS_WH				0x00044003	///<
#define BCM2835_VC_TAG_SET_PHYS_WH				0x00048003	///<
#define BCM2835_VC_TAG_GET_VIRT_WH				0x00040004	///<
#define BCM2835_VC_TAG_TEST_VIRT_WH				0x00044004	///<
#define BCM2835_VC_TAG_SET_VIRT_WH				0x00048004	///<
#define BCM2835_VC_TAG_GET_DEPTH				0x00040005	///<
#define BCM2835_VC_TAG_TEST_DEPTH				0x00044005	///<
#define BCM2835_VC_TAG_SET_DEPTH				0x00048005	///<
#define BCM2835_VC_TAG_GET_PIXEL_ORDER			0x00040006	///<
#define BCM2835_VC_TAG_TEST_PIXEL_ORDER			0x00044006	///<
#define BCM2835_VC_TAG_SET_PIXEL_ORDER			0x00048006	///<
#define BCM2835_VC_TAG_GET_ALPHA_MODE			0x00040007	///<
#define BCM2835_VC_TAG_TEST_ALPHA_MODE			0x00044007	///<
#define BCM2835_VC_TAG_SET_ALPHA_MODE			0x00048007	///<
#define BCM2835_VC_TAG_GET_PITCH				0x00040008	///<
#define BCM2835_VC_TAG_GET_VIRT_OFFSET			0x00040009	///<
#define BCM2835_VC_TAG_TEST_VIRT_OFFSET			0x00044009	///<
#define BCM2835_VC_TAG_SET_VIRT_OFFSET			0x00048009	///<
#define BCM2835_VC_TAG_GET_OVERSCAN				0x0004000a	///<
#define BCM2835_VC_TAG_TEST_OVERSCAN			0x0004400a	///<
#define BCM2835_VC_TAG_SET_OVERSCAN				0x0004800a	///<
#define BCM2835_VC_TAG_GET_PALETTE				0x0004000b	///<
#define BCM2835_VC_TAG_TEST_PALETTE				0x0004400b	///<
#define BCM2835_VC_TAG_SET_PALETTE				0x0004800b	///<

typedef enum {
	BCM2835_MAILBOX_POWER_CHANNEL 	= 0,	///< For use by the power management interface
	BCM2835_MAILBOX_FB_CHANNEL 		= 1,	///< https://github.com/raspberrypi/firmware/wiki/Mailbox-framebuffer-interface
	BCM2835_MAILBOX_VCHIQ_CHANNEL 	= 3,    ///< For use by the VCHIQ interface
	BCM2835_MAILBOX_PROP_CHANNEL 	= 8		///< https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
} bcm2835MailboxChannels;

#define BCM2835_MAILBOX_SUCCESS	(uint32_t)0x80000000	///< Request successful
#define BCM2835_MAILBOX_ERROR	(uint32_t)0x80000001	///< Error parsing request buffer (partial response)

#define BCM2835_MAILBOX_STATUS_WF	0x80000000	///< Write full
#define	BCM2835_MAILBOX_STATUS_RE	0x40000000	///< Read empty



u32 mailbox_read(int channel);
void mailbox_send(int channel, u32 data);


#endif
