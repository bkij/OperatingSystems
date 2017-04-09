#include <string.h>
#include "contact_info.h"

int by_surname(ContactInfo *left, ContactInfo *right)
{
    return strncmp(left->surname, right->surname, sizeof(left->surname - 1));
}

int by_birthdate(ContactInfo *left, ContactInfo *right)
{
    char year_left[4];
    char year_right[4];
    char month_left[2];
    char month_right[2];
    char day_left[2];
    char day_right[2];
    strncpy(year_left, left->birthdate + 4, 4);
    strncpy(year_right, right->birthdate + 4, 4);
    strncpy(month_left, left->birthdate + 2, 2);
    strncpy(month_right, right->birthdate + 2, 2);
    strncpy(day_left, left->birthdate, 2);
    strncpy(day_right, right->birthdate, 2);

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
    return strncmp(left->mail, right->mail, sizeof(left->mail - 1));
}

int by_phone(ContactInfo *left, ContactInfo *right)
{
    return strncmp(left->phone, right->phone, sizeof(left->phone - 1));
}
