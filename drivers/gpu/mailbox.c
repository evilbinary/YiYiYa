#include "mailbox.h"

mail_message_t mailbox_read(int channel) {
    mail_status_t stat;
    mail_message_t res;
    // Make sure that the message is from the right channel
    do {
        // Make sure there is mail to recieve
        do {
            stat = *MAIL0_STATUS;
        } while (stat.empty);
        // Get the message
        res = *MAIL0_READ;
    } while (res.channel != channel);
    return res;
}

void mailbox_send(mail_message_t msg, int channel) {
    mail_status_t stat;
    msg.channel = channel;
    // Make sure you can send mail
    do {
        stat = *MAIL0_STATUS;
    } while (stat.full);
    // send the message
    *MAIL0_WRITE = msg;
}

mail_message_t mailbox_send_data(mail_message_t message,int channel) {
    mailbox_send(message,channel);
    mail_message_t data = mailbox_read(channel);
    return data;
}