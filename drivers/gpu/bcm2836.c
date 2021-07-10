#include "bcm2836.h"

#include "drivers/vga/vga.h"

static uint32_t get_value_buffer_len(property_message_tag_t * tag) {
    switch(tag->proptag) {
        case FB_ALLOCATE_BUFFER: 
        case FB_GET_PHYSICAL_DIMENSIONS:
        case FB_SET_PHYSICAL_DIMENSIONS:
        case FB_GET_VIRTUAL_DIMENSIONS:
        case FB_SET_VIRTUAL_DIMENSIONS:
            return 8;
        case FB_GET_BITS_PER_PIXEL:
        case FB_SET_BITS_PER_PIXEL:
        case FB_GET_BYTES_PER_ROW:
            return 4;
        case FB_RELESE_BUFFER:
        default:
            return 0;
    }
}

static void write_pixel(vga_device_t* vga, uint32_t x, uint32_t y,
                 const pixel_t* pix) {
  uint8_t* location = vga->frambuffer + y * vga->bpp + x * BYTES_PER_PIXEL;
  memcpy(location, pix, BYTES_PER_PIXEL);
}

int bcm2836_init(vga_device_t* vga) {
  mail_message_t mail;
  property_message_buffer_t * msg=kmalloc_alignment(sizeof(property_message_buffer_t),16);
  mail.channel=MAILBOX_CHANNEL_PROPERTY_TAGS_ARM_TO_VC;
  mail.data=msg;
  property_message_tag_t tags[5];

  //set phisyic width height
  tags[0].proptag = FB_SET_PHYSICAL_DIMENSIONS;
  tags[0].value_buffer.fb_screen_size.width = vga->width;
  tags[0].value_buffer.fb_screen_size.height = vga->height;

 //set virtual widht height
  tags[1].proptag = FB_SET_VIRTUAL_DIMENSIONS;
  tags[1].value_buffer.fb_screen_size.width = vga->width;
  tags[1].value_buffer.fb_screen_size.height = vga->height;

  //set bit per pixe
  tags[2].proptag = FB_SET_BITS_PER_PIXEL;
  tags[2].value_buffer.fb_bits_per_pixel = vga->bpp;
  tags[3].proptag = NULL_TAG;

  //tag size  
  for (int i = 0; tags[i].proptag != NULL_TAG&&i<5; i++) {
    msg->size += get_value_buffer_len(&tags[i]) + 3*sizeof(uint32_t);          
  }
  msg->size += 3*sizeof(uint32_t); 

  msg->req_res_code=REQUEST;
  msg->tags=tags;

  //init
  mailbox_send_data(mail,MAILBOX_CHANNEL_PROPERTY_TAGS_ARM_TO_VC);

  if(msg->req_res_code != RESPONSE_SUCCESS ) {
    kprintf("int bcm error %d\n",msg->req_res_code);
    return -1;
  }

  tags[0].proptag = FB_ALLOCATE_BUFFER;
  tags[0].value_buffer.fb_screen_size.width = 0;
  tags[0].value_buffer.fb_screen_size.height = 0;
  tags[0].value_buffer.fb_allocate_align = 16;
  tags[1].proptag = NULL_TAG;
  
  for (int i = 0; tags[i].proptag != NULL_TAG&&i<5; i++) {
    msg->size += get_value_buffer_len(&tags[i]) + 3*sizeof(uint32_t);          
  }
  msg->size += 3*sizeof(uint32_t); 
  msg->req_res_code=REQUEST;
  msg->tags=tags;
  mail.channel=MAILBOX_CHANNEL_PROPERTY_TAGS_ARM_TO_VC;
  mail.data=msg;
  
  //frambuffer
  mailbox_send_data(mail,MAILBOX_CHANNEL_PROPERTY_TAGS_ARM_TO_VC);
  if (msg->req_res_code != RESPONSE_SUCCESS ) {
    kprintf("int bcm error 2\n");
    return -1;
  }


  vga->frambuffer = (u32)tags[0].value_buffer.fb_allocate_res.fb_addr & 0x3fffffff;
  vga->framebuffer_length = tags[0].value_buffer.fb_allocate_res.fb_size;

  return 0;
}

int gpu_init_mode(vga_device_t* vga, int mode) {
  if (mode == VGA_MODE_80x25) {
    vga->width = 80;
    vga->height = 25;
  } else if (mode == VGA_MODE_320x200x256) {
    vga->width = 320;
    vga->height = 256;
  } else if (mode == VGA_MODE_640x480x24) {
    vga->width = 640;
    vga->height = 480;
    vga->bpp = 24;
  } else if (mode == VGA_MODE_1024x768x32) {
    vga->width = 1024;
    vga->height = 768;
    vga->bpp = 32;
  }
  vga->mode = mode;
  vga->write = NULL;

  map_page(MAILBOX_BASE, MAILBOX_BASE, 0);

  bcm2836_init(vga);

  vga->framebuffer_index = 0;
  vga->framebuffer_count = 1;
  kprintf("fb addr:%x len:%x\n", vga->frambuffer, vga->framebuffer_length);
  u32 addr = 0;
  for (int i = 0; i < vga->framebuffer_length / PAGE_SIZE; i++) {
    map_page(addr, addr, 0);
    addr += 0x1000;
  }

  // static const pixel_t BLACK = {0xff, 0xff, 0xff};
  // for (uint32_t j = 0; j < vga->height; j++) {
  //   for (uint32_t i = 0; i < vga->width; i++) {
  //     write_pixel(vga, i, j, &BLACK);
  //   }
  // }
  return 0;
}