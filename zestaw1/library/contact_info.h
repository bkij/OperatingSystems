#ifndef CONTACT_INFO_H
#define CONTACT_INFO_H

/*
 *  The following structure assumes Polish addresses
 *  
 *  The longest city name in Poland has 30 characters
 *
 *  Street number assumes the numbers themself are no more
 *  than 4 digits long + a letter + \0
 *
 *  Postal code format: xx-xxx\0
 */
typedef struct Address {
    char street_name[256];
    char street_number[6];
    char postal_code[7];
    char city[31];
} Address;

/*
 * Date of birth in DDMMYYYY\0 format 
 * Telephone number in xxxxxxxxx\0 format
 */
typedef struct ContactInfo {
    char name[20];
    char surname[20];
    char birthdate[9];
    char mail[256];
    char phone[10];
    Address address;
} ContactInfo;

/*
 * Comparators
 */
int by_surname(ContactInfo *left, ContactInfo *right);
int by_birthdate(ContactInfo *left, ContactInfo *right);
int by_mail(ContactInfo *left, ContactInfo *right);
int by_phone(ContactInfo *left, ContactInfo *right);

#endif // CONTACT_INFO_H
