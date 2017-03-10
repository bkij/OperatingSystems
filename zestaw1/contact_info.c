#include <string.h>
#include "contact_info.h"

int by_surname(ContactInfo *left, ContactInfo *right)
{
    return strncmp(left->name, right->name, sizeof(left->name - 1));
}

int by_birthdate(ContactInfo *left, ContactInfo *right)
{
    char year_left[4];
    char year_right[4];
    char month_left[2];
    char month_right[2];
    char day_left[2];
    char day_right[2];
    strncpy(year_left, left->date_of_birth[4], 4);
    strncpy(year_right, right->date_of_birth[4], 4);
    strncpy(month_left, left->date_of_birth[2], 2);
    strncpy(month_rigth, right->date_of_birth[2], 2);
    strncpy(day_left, left->date_of_birth, 2);
    strncpy(day_right, right->date_of_birth, 2);

    if(strncmp(year_left, year_right, 4) != 0) {
        return strncmp(year_left, year_right, 4);
    }
    if(strncmp(month_left, month_right, 2) != 0) {
        return strncmp(month_left, month_right, 2);
    }
    return strncmp(day_left, day_right, 2);
}

int by_mail(ContactInfo *left, ContactInfo *right)
{
    return strncmp(left->email, right->email, sizeof(left->email - 1));
}

int by_phone(ContactInfo *left, ContactInfo *right)
{
    return strncmp(left->telephone_number, right->telephone_number, sizeof(left->telephone_number - 1));
}
