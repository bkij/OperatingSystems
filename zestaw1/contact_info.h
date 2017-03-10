#ifndef CONTACT_INFO_H
#define CONTACT_INFO_H

/*
 *  The following structure assumes Polish addresses
 *  
 *  The longest city name in Poland has 30 characters
 *
 *  Street number assumes the numbers themself are no more
 *  than 4 digits long + a letter
 */
typedef struct Address {
    char street_name[256];
    char street_number[5];
    char postal_code[6];
    char city[31];
} Address;

/*
 * Date of birth in DDMMYYYY format
 */
typedef struct ContactInfo {
    char name[20];
    char surname[20];
    char date_of_birth[8];
    char email[256];
    char telephone_number[9];
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
