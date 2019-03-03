/*	Tyler Green
//	tgreen2
//	CIS 415 Project 0
//	
//	This is my own work except some logic and organization were taken from Lectures by Joe Sventek. Examples from the lecture are what I used as guidance for this ADT
*/

#include "date.h"
#include "stdlib.h"
#include "stdio.h"
#include "iterator.h"
#include "string.h"

typedef struct data
{
	char* day;
	char* month;
	char* year;
} Data;

static const Date* dt_duplicate(const Date *d)
{
	Date *newDate = (Date*)malloc(sizeof(Date));
	Data *newData = (Data*)malloc(sizeof(Data));
	Data *oldData = (Data *)(d->self);	

	newData->day = oldData->day;
	newData->month = oldData->month;
	newData->year = oldData->year;

	newDate->self = newData;

	return newDate;
}

static int dt_compare(const Date *date1, const Date *date2)
{
	Data * data1 = (Data*)(date1->self);
	Data * data2 = (Data*)(date2->self);
	int yy1 = atoi(data1->year);
	int yy2 = atoi(data2->year);
	int mm1 = atoi(data1->month);
	int mm2 = atoi(data2->month);
	int dd1 = atoi(data1->day);
	int dd2 = atoi(data2->day);

	if (yy1 == yy2)
	{
		if (mm1 == mm2)
		{
			if (dd1 == dd2)
				return 0;
			return(dd1 < dd2 ? -1 : 1);
		}	
		else
			return(mm1 < mm2 ? -1 : 1); 
	}
	else
		return(yy1 < yy2 ? -1 : 1);
}

static void dt_destroy(const Date *d)
{
	Data * dtda = (Data *)d->self;
	free(dtda->day);
	free(dtda->month);
	free(dtda->year);
	free(dtda);
	free((void*)d);
}


static Date template = {NULL, dt_duplicate, dt_compare, dt_destroy};

const Date *Date_create(char *datestr)
{
	Date *dt = (Date*)malloc(sizeof(Date));
	if (dt!=NULL)
	{
		Data *dtda = (Data*)malloc(sizeof(Data));
		if (dtda != NULL)
		{
			char dd[540], mm[540], yy[540];

			sscanf(datestr, "%[^/]/%[^/]/%[^ ]", dd, mm, yy);
			dtda->day = strdup(dd);
			dtda->month = strdup(mm);
			dtda->year = strdup(yy);
			*dt = template;
			dt->self = (void *)dtda;
		}
		else 
		{
			free(dt);
			return NULL;
		}
	}
	return dt;
}
