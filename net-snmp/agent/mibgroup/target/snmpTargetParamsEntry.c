/* TargetParamTable MIB

   This file was generated by mib2c and is intended for use as a mib module
   for the ucd-snmp snmpd agent. Edited by Michael Baer 

   last changed 2/2/99.
*/

#include <config.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#include "mibincl.h"
#include "snmpTargetParamsEntry.h"
#include "read_config.h"
#include "callback.h"

#define snmpTargetParamsOIDLen 11  /*This is base+column, 
				     i.e. everything but index*/

oid snmpTargetParamsOID[snmpTargetParamsOIDLen] = {1,3,6,1,6,3,15,1,3,1,0};

static struct targetParamTable_struct *aPTable=0;


/* Utility routines */


/* TargetParamTable_create creates and returns a pointer
   to a targetParamTable_struct with default values set */
struct targetParamTable_struct 
*snmpTargetParamTable_create(void)
{
  struct targetParamTable_struct *newEntry;

  newEntry = (struct targetParamTable_struct *)
    malloc(sizeof(struct targetParamTable_struct));

  newEntry->paramName   = 0;
  newEntry->mpModel     = -1;
 
  newEntry->secModel    = -1;
  newEntry->secName     = 0;
  newEntry->secLevel    = -1;
  
  newEntry->storageType = SNMP_STORAGE_NONVOLATILE;
  newEntry->rowStatus   = SNMP_ROW_NONEXISTENT;
  newEntry->next        = 0;
  return newEntry;
}


/* TargetParamTable_dispose frees the space allocated to a
   targetParamTable_struct */
void snmpTargetParamTable_dispose(
     struct targetParamTable_struct *reaped)
{
  free(reaped->paramName);
  free(reaped->secName);
  
  free(reaped);
}  /* snmpTargetParamTable_dispose  */


/* snmpTargetParamTable_addToList adds a targetParamTable_struct 
   to a list passed in. The list is assumed to be in a sorted order,
   low to high and this procedure inserts a new struct in the proper 
   location. Sorting uses OID values based on paramName. A new equal value 
   overwrites a current one. */
void snmpTargetParamTable_addToList(
     struct targetParamTable_struct *newEntry,
     struct targetParamTable_struct **listPtr)
{
  static struct targetParamTable_struct *curr_struct, *prev_struct;
  int    i;
  size_t newOIDLen = 0, currOIDLen = 0;
  oid    newOID[128], currOID[128];
  
  /* if the list is empty, add the new entry to the top */
  if ( (prev_struct = curr_struct = *listPtr) == 0 ) {
    *listPtr = newEntry;
    return;
  }
  else {
    /* get the 'OID' value of the new entry */
    newOIDLen = strlen(newEntry->paramName);
    for(i=0; i < newOIDLen ;i++) {
      newOID[i] = newEntry->paramName[i];
    }

    /* search through the list for an equal or greater OID value */
    while (curr_struct != 0) {
      currOIDLen = strlen(curr_struct->paramName);
      for(i=0; i < currOIDLen ;i++) {
	currOID[i] = curr_struct->paramName[i];
      }

      i=snmp_oid_compare(newOID, newOIDLen, currOID, currOIDLen);
      if (i==0) {  /* Exact match, overwrite with new struct */
	newEntry->next = curr_struct->next;
	/* if curr_struct is the top of the list */
	if (*listPtr == curr_struct)  *listPtr = newEntry;
	else prev_struct->next = newEntry;
	snmpTargetParamTable_dispose(curr_struct);
	return;
      }
      else if (i < 0) { /* Found a greater OID, insert struct in front of it.*/
	newEntry->next = curr_struct;
	/* if curr_struct is the top of the list */
	if (*listPtr == curr_struct) *listPtr = newEntry;
	else prev_struct->next = newEntry;
	return;
      }
      prev_struct = curr_struct;
      curr_struct = curr_struct->next;
    }
  }
  /* if we're here, no larger OID was ever found, insert on end of list */
  prev_struct->next = newEntry;
}  /* snmpTargeParamTable_addToList  */


/* snmpTargetParamTable_remFromList removes a targetParamTable_struct 
   from the list passed in */
void snmpTargetParamTable_remFromList(
     struct targetParamTable_struct *oldEntry,
     struct targetParamTable_struct **listPtr)
{
  struct targetParamTable_struct *tptr;

  if ( (tptr = *listPtr) == 0 ) return;
  else if (tptr == oldEntry) {
    *listPtr = (*listPtr)->next;
    snmpTargetParamTable_dispose(tptr);
    return;
  }
  else  {
    while (tptr->next != 0) {
      if (tptr->next == oldEntry) {
	tptr->next = tptr->next->next;
	snmpTargetParamTable_dispose(oldEntry);
	return;
      }
      tptr = tptr->next;
    }
  }	
}  /* snmpTargetParamTable_remFromList  */


/* lookup OID in the link list of Table Entries */
struct targetParamTable_struct *
search_snmpTargetParamsTable(
     oid    *baseName,
     size_t baseNameLen,
     oid    *name,
     size_t *length,
     int    exact)
{
   static struct targetParamTable_struct *temp_struct;
   int    i;
   size_t myOIDLen = 0;
   oid    newNum[128];

   /* lookup entry in p / * Get Current MIB ID */
   memcpy(newNum, baseName, baseNameLen*sizeof(oid));
  
   for( temp_struct = aPTable; temp_struct != 0; temp_struct = temp_struct->next) {
     for(i=0; i < strlen(temp_struct->paramName) ;i++) {
       newNum[baseNameLen+i] = temp_struct->paramName[i];
     }
     myOIDLen = baseNameLen+strlen(temp_struct->paramName);
     i=snmp_oid_compare(name, *length, newNum, myOIDLen);
     /* Assumes that the linked list sorted by OID, low to high */
     if ( (i==0 && exact!=0) || (i<0 && exact==0) ) {
       if (exact == 0) {
	 memcpy(name, newNum, myOIDLen*sizeof(oid));
	 *length = myOIDLen;
       }
       return temp_struct;
     }
   }
   return(0);
}  /* search_snmpTargetParamsTable */


/* snmpTargetParams_rowStatusCheck is boolean funciton that  checks 
   the status of a row's values in order to determine whether
   the row should be notReady or notInService  */
int snmpTargetParams_rowStatusCheck(
     struct targetParamTable_struct *entry)
{
  if ( (entry->mpModel  < 0) || (entry->secModel < 0) ||
       (entry->secLevel < 0) || (entry->secName == 0)   )
    return 0;
  else
    return 1;
}  /* snmtpTargetParamTable_rowStatusCheck */


/* initialization routines */

void 
init_snmpTargetParamsEntry(void)
{
  aPTable = 0;
  snmpd_register_config_handler("targetParams", snmpd_parse_config_targetParams,0,"");

  /* we need to be called back later */
  snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                         store_snmpTargetParamsEntry, NULL);
}  /*  init_snmpTargetParmsEntry  */


int snmpTargetParams_addParamName(
     struct targetParamTable_struct *entry,
     char   *cptr)
{
  size_t len;
  if (cptr == 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: no param name in config string\n"));
    return(0);
  }
  else {
    len = strlen(cptr);    
    /* spec check for string 1-32 */
    if (len < 1 || len > 32)  {
      DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: param name out of range in config string\n"));
      return(0);
    }
    entry->paramName = (char *)malloc(sizeof(char) * (len + 1));
    strncpy(entry->paramName, cptr, len);
    entry->paramName[len] = '\0';
  }
  return(1);
}
  

int snmpTargetParams_addMPModel(
     struct targetParamTable_struct *entry,
     char   *cptr)
{
  if (cptr == 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: no mp model in config string\n"));
    return(0);
  }
  else if (!(isdigit(*cptr))) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargeParamsEntry: mp model is not digit in config string\n"));
    return(0);
  }
  /* spec check MP Model >= 0 */
  else if ( (entry->mpModel = (int)strtol(cptr, (char **)NULL, 0)) < 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargeParamsEntry: mp model out of range in config string\n"));
    return(0);
  }
  return(1);
}  /* snmpTargetParams_addMPModel  */
  

int snmpTargetParams_addSecModel(
     struct targetParamTable_struct *entry,
     char   *cptr)
{
  if (cptr == 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: no sec model in config string\n"));
    return(0);
  }
  else if (!(isdigit(*cptr))) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargeParamsEntry: security model is not digit in config string\n"));
    return(0);
  }
  /* spec check Sec. Model > 0 */
  else if ( (entry->secModel = (int)strtol(cptr, (char **)NULL, 0)) <= 0) {
      DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: security model out of range in config string\n"));
      return(0);
  }
  return(1);
}  /*  snmpTargetParams_addSecModel  */


int snmpTargetParams_addSecName(
     struct targetParamTable_struct *entry,
     char   *cptr)
{
  size_t len;
  if (cptr == 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: no security name in config string\n"));
    return(0);
  }
  else {
    len = strlen(cptr);
    entry->secName = (char *)malloc(sizeof(char)*(len+1));
    strncpy(entry->secName, cptr, len);
    entry->secName[len] = '\0';
  }
  return(1);
}  /* snmpTargetParams_addSecName  */


int snmpTargetParams_addSecLevel(
     struct targetParamTable_struct *entry,
     char   *cptr)
{
  if (cptr == 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: no security level in config string\n"));
    return(0);
  }
  else if (!(isdigit(*cptr)))  {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargeParamsEntry: security level is not digit in config string\n"));
    return(0);
   }
  /* no spec range check, but noAuthNoPriv is 1 so... */
  else if ( (entry->secLevel = (int)strtol(cptr, (char **)NULL, 0)) <= 0 ) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargeParamsEntry: security level is not greater than 0 in config string\n"));
    return(0);
  }
  return(1);
}  /*  snmpTargetParams_addSecLevel  */
  

int snmpTargetParams_addStorageType(
     struct targetParamTable_struct *entry,
     char   *cptr)
{
  if (cptr == 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: no storage type in config string\n"));
    return(0);
  }
  else if (!(isdigit(*cptr))) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargeParamsEntry: storage type is not digit in config string\n"));
    return(0);
  }
  /* check that storage type is a possible value */
  else if ( ((entry->storageType = (int)strtol(cptr, (char **)NULL, 0)) 
	     != SNMP_STORAGE_OTHER) &&
	    (entry->storageType != SNMP_STORAGE_VOLATILE) && 
	    (entry->storageType != SNMP_STORAGE_NONVOLATILE)  &&
	    (entry->storageType != SNMP_STORAGE_PERMANENT) && 
	    (entry->storageType != SNMP_STORAGE_READONLY) )  {
    DEBUGMSGTL(("snmpTargetParamsEntry", "ERROR snmpTargeParamsEntry: storage type is not a valid value of"));
    DEBUGMSG(("snmpTargetParamsEntry", " other(%d), volatile(%d), nonvolatile(%d), permanent(%d), or ",
	   SNMP_STORAGE_OTHER, SNMP_STORAGE_VOLATILE, SNMP_STORAGE_NONVOLATILE,
	   SNMP_STORAGE_PERMANENT));
    DEBUGMSGTL(("snmpTargetParamsEntry","readonly(%d) in config string.\n", SNMP_STORAGE_READONLY));

    return(0);
  }
  return(1);
}  /* snmpTargetParams_addStorageType  */


int snmpTargetParams_addRowStatus(
     struct targetParamTable_struct *entry,
     char   *cptr)
{
  if (cptr == 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargetParamsEntry: no row status in config string\n"));
    return(0);
  }
  else if (!(isdigit(*cptr)))  {
    DEBUGMSGTL(("snmpTargetParamsEntry","ERROR snmpTargeParamsEntry: row status is not digit in config string\n"));
    return(0);
  }
  /* check that row status is a valid value */
  else if ( ((entry->rowStatus = (int)strtol(cptr, (char **)NULL, 0)) 
	     != SNMP_ROW_ACTIVE) &&
	    (entry->rowStatus != SNMP_ROW_NOTINSERVICE) &&
	    (entry->rowStatus != SNMP_ROW_NOTREADY) ) {
    DEBUGMSGTL(("snmpTargetParamsEntry", "ERROR snmpTargetParamsEntry: Row Status is not a valid value of "));
    DEBUGMSG(("snmpTargetParamsEntry", "active(%d), notinservice(%d), or notready(%d) in config string.\n",
	   SNMP_ROW_ACTIVE, SNMP_ROW_NOTINSERVICE, SNMP_ROW_NOTREADY));

    return(0);
  }
  return(1);
}  /* snmpTargetParams_addRowStatus  */
    

void snmpd_parse_config_targetParams(
     char *token, char *char_ptr)
{
  char *cptr = char_ptr, buff[1024];
  struct targetParamTable_struct *newEntry;

  newEntry = snmpTargetParamTable_create();
  
  cptr = copy_word(cptr, buff);
  if (snmpTargetParams_addParamName(newEntry, buff) == 0) {
    snmpTargetParamTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  if (snmpTargetParams_addMPModel(newEntry, buff) == 0) {
    snmpTargetParamTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  if (snmpTargetParams_addSecModel(newEntry, buff) == 0) {
    snmpTargetParamTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  if (snmpTargetParams_addSecName(newEntry, buff) == 0) {
    snmpTargetParamTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  if (snmpTargetParams_addSecLevel(newEntry, buff) == 0) {
    snmpTargetParamTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  if (snmpTargetParams_addStorageType(newEntry, buff) == 0) {
    snmpTargetParamTable_dispose(newEntry);
    return;
  }
  cptr = copy_word(cptr, buff);
  if (snmpTargetParams_addRowStatus(newEntry, buff) == 0) {
    snmpTargetParamTable_dispose(newEntry);
    return;
  }
  sprintf(buff, "snmp_parse_config_targetParams, read: %s %d %d %s %d %d %d\n",
	  newEntry->paramName, newEntry->mpModel,  newEntry->secModel, 
	  newEntry->secName,   newEntry->secLevel, newEntry->storageType,
	  newEntry->rowStatus);
  DEBUGMSGTL(("snmpTargetParamsEntry", buff));

  snmpTargetParamTable_addToList(newEntry, &aPTable);
} /* snmpd_parse_config_target */


/* shutdown routines */


/* store_snmpTargetParamsEntry handles the presistent storage proccess 
   for this MIB table. It writes out all the non-volatile rows 
   to permanent storage on a shutdown  */
int 
store_snmpTargetParamsEntry(int majorID, int minorID, void *serverarg,
                            void *clientarg)
{
  struct targetParamTable_struct *curr_struct;
  char line[1024];

  strcpy(line, "");
  if ( (curr_struct = aPTable) != 0) {
    while (curr_struct != 0) {
      if ( (curr_struct->storageType == SNMP_STORAGE_NONVOLATILE || 
	    curr_struct->storageType == SNMP_STORAGE_PERMANENT) 
	                        &&
	   (curr_struct->rowStatus == SNMP_ROW_ACTIVE ||
	    curr_struct->rowStatus == SNMP_ROW_NOTINSERVICE) ) {
	sprintf(&line[strlen(line)], "targetParams %s %i %i %s %i %i %i\n",
			 curr_struct->paramName, curr_struct->mpModel,
			 curr_struct->secModel,  curr_struct->secName,
			 curr_struct->secLevel,  curr_struct->storageType,
			 curr_struct->rowStatus  );

	/* store to file */
	snmpd_store_config(line);
      }
      curr_struct = curr_struct->next;
    }
  }
  return SNMPERR_SUCCESS;
}  /*  store_snmpTargetParmsEntry  */


/* MIB table access routines */


u_char *
var_snmpTargetParamsEntry(
    struct  variable *vp,
    oid     *name,
    size_t  *length,
    int     exact,
    size_t  *var_len,
    WriteMethod **write_method)
{
  /* variables we may use later */
  static long long_ret;
  static unsigned char string[1500];
  struct targetParamTable_struct *temp_struct;
  
  *write_method = 0;           /* assume it isnt writable for the time being */
  *var_len = sizeof(long_ret); /* assume an integer and change later if not */
  
  /* look for OID in current table */
  if ( (temp_struct = search_snmpTargetParamsTable(vp->name, vp->namelen, 
				      name, length, exact)) == 0 ) {
    if (vp->magic == SNMPTARGETPARAMSROWSTATUS)  {
      *write_method = write_snmpTargetParamsRowStatus;
    }
    return(0);
  }
  
  /* We found what we were looking for, either the next OID or the exact OID */
  /* this is where we do the value assignments for the mib results. */
  switch(vp->magic) {

    case SNMPTARGETPARAMSMPMODEL:
      *write_method = write_snmpTargetParamsMPModel;
      /* if unset value, (i.e. new row) */
      if (temp_struct->mpModel == -1)  return(0);
      long_ret = temp_struct->mpModel;
      return (unsigned char *) &long_ret;

    case SNMPTARGETPARAMSSECURITYMODEL:
      *write_method = write_snmpTargetParamsSecurityModel;
      /* if unset value, (i.e. new row) */
      if (temp_struct->secModel == -1)  return(0);
      long_ret = temp_struct->secModel;
      return (unsigned char *) &long_ret;

    case SNMPTARGETPARAMSSECURITYNAME:
      *write_method = write_snmpTargetParamsSecurityName;
      /* if unset value, (i.e. new row) */
      if (temp_struct->secName == 0)  return(0);
      /* including null character. */
      memcpy(string, temp_struct->secName, 
	     strlen(temp_struct->secName)*sizeof(char));
      string[strlen(temp_struct->secName)] = '\0';
      *var_len = strlen(temp_struct->secName);
      return (unsigned char *) string;

    case SNMPTARGETPARAMSSECURITYLEVEL:
      *write_method = write_snmpTargetParamsSecurityLevel;
      /* if unset value, (i.e. new row) */
      if (temp_struct->secLevel == -1)  return(0);
      long_ret = temp_struct->secLevel;
      return (unsigned char *) &long_ret;

    case SNMPTARGETPARAMSSTORAGETYPE:
      *write_method = write_snmpTargetParamsStorageType;
      long_ret = temp_struct->storageType;
      return (unsigned char *) &long_ret;

    case SNMPTARGETPARAMSROWSTATUS:
      *write_method = write_snmpTargetParamsRowStatus;
      long_ret = temp_struct->rowStatus;
      return (unsigned char *) &long_ret;

    default:
      DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_snmpTargetParamsEntry\n", vp->magic));
  }
  return 0;
}  /* var_snmpTargetParamsEntry */


/* Assign a value to the mpModel variable */
int
write_snmpTargetParamsMPModel(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  static long                    long_ret;
  size_t                         size;
  size_t                         bigsize=1000;
  struct targetParamTable_struct *temp_struct;

  /* check incoming variable */
  if (var_val_type != ASN_INTEGER) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsMPModel : not ASN_INTEGER\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsMPModel : bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);
  /* spec check range */
  if (long_ret < 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsMPModel : MP Model out of range\n"));
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Find row in linked list and check pertinent status... */
  snmpTargetParamsOID[snmpTargetParamsOIDLen-1] = SNMPTARGETPARAMSMPMODELCOLUMN;
  if ((temp_struct = search_snmpTargetParamsTable(snmpTargetParamsOID, snmpTargetParamsOIDLen, 
				     name, &name_len, 1)) == 0 ) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsMPModel : BAD OID\n"));
    return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamMPModel : row is read only\n"));
    return SNMP_ERR_READONLY;
  }
  /* check if row is active */
  if (temp_struct->rowStatus == SNMP_ROW_ACTIVE) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsMPModel : This change not allowed in active row.\n"));
    return SNMP_ERR_INCONSISTENTVALUE;
  }
  /* Finally, we're golden, should we save value? */
  if (action == COMMIT)  {
    temp_struct->mpModel = long_ret;
    /* If row is new, check if its status can be updated */
    if ( (temp_struct->rowStatus == SNMP_ROW_NOTREADY) &&
	 (snmpTargetParams_rowStatusCheck(temp_struct) != 0) )
      temp_struct->rowStatus = SNMP_ROW_NOTINSERVICE;
  }
  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetParamsMPModel */


/* Assign a value to the Security Model variable */
int
write_snmpTargetParamsSecurityModel(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  static long                    long_ret;
  size_t                         size;
  size_t                         bigsize=1000;
  struct targetParamTable_struct *temp_struct;

  /* check incoming variable */
  if (var_val_type != ASN_INTEGER) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityModel : not ASN_INTEGER\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityModel : bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);
  /* spec check range */
  if (long_ret <= 0)  {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecModel : Security Model out of range\n"));
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Find struct in linked list and check row status */
  snmpTargetParamsOID[snmpTargetParamsOIDLen-1] = SNMPTARGETPARAMSSECURITYMODELCOLUMN;
  if ((temp_struct = search_snmpTargetParamsTable(snmpTargetParamsOID, snmpTargetParamsOIDLen, 
				     name, &name_len, 1)) == 0 ) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamSecurityModel : BAD OID!\n"));
      return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamSecurityModel : row is read only\n"));
    return SNMP_ERR_READONLY;
  }
  /* check if row active */
  if (temp_struct->rowStatus == SNMP_ROW_ACTIVE) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamSecurityModel : This change not allowed in active row.\n"));
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Finally, we're golden, check if we should save value */
  if (action == COMMIT)  {    
    temp_struct->secModel = long_ret;
    /* If row is new, check if its status can be updated */
    if ( (temp_struct->rowStatus == SNMP_ROW_NOTREADY) &&
	 (snmpTargetParams_rowStatusCheck(temp_struct) != 0) )
      temp_struct->rowStatus = SNMP_ROW_NOTINSERVICE;
  }

  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetParamsSecurityModel */


/* Assign a value to the Security Name variable */
int
write_snmpTargetParamsSecurityName(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  static unsigned char           string[1500];
  size_t                         size=1500;
  size_t                         bigsize=1000;
  struct targetParamTable_struct *temp_struct;

  /* check incoming variable */
  if (var_val_type != ASN_OCTET_STR) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityName : not ASN_OCTET_STR\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(string))) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityName : bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }

  /* spec check, 0-255, this means EMPTY STRINGS ALLOWED! */
  size = sizeof(string);
  asn_parse_string(var_val, &bigsize, &var_val_type, string, &size);
  if (size > 255 || size < 0) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityName : bad length\n"));
    return SNMP_ERR_WRONGLENGTH;
  }

  /* Find the struct in the linked list and check row status */
  snmpTargetParamsOID[snmpTargetParamsOIDLen-1] = SNMPTARGETPARAMSSECURITYNAMECOLUMN;
  if ((temp_struct = search_snmpTargetParamsTable(snmpTargetParamsOID, snmpTargetParamsOIDLen, 
				     name, &name_len, 1)) == 0 ) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityName : BAD OID!\n"));
    return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityName : row is read only\n"));
    return SNMP_ERR_READONLY;
  }
  /* check if row active */
  if (temp_struct->rowStatus == SNMP_ROW_ACTIVE) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityName : This change not allowed in active row.\n"));
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Finally, we're golden, check if we should save value */
  if (action == COMMIT)  {    
    free(temp_struct->secName);
    temp_struct->secName = (char *)malloc( (size*sizeof(char))+1 );
    memcpy(temp_struct->secName, string, size*sizeof(char));
    temp_struct->secName[size] = '\0';
    
    /* If row is new, check if its status can be updated */
    if ( (temp_struct->rowStatus == SNMP_ROW_NOTREADY) &&
	 (snmpTargetParams_rowStatusCheck(temp_struct) != 0) )
      temp_struct->rowStatus = SNMP_ROW_NOTINSERVICE;
  }
  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetParamsSecurityName */


int
write_snmpTargetParamsSecurityLevel(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  static long                    long_ret;
  size_t                         size;
  size_t                         bigsize=1000;
  struct targetParamTable_struct *temp_struct;
 
  /* check incoming variable */
  if (var_val_type != ASN_INTEGER) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityLevel : not ASN_INTEGER\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityLevel : bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);
  /* spec check, no spec check, but noAuthNoPriv is 1 so... */
  if (long_ret <= 0 )  {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargeParamsSecurityLevel: security level is not noAuthNoPriv(1) or higher\n"));
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Find the struct in linked list and check its status */
  snmpTargetParamsOID[snmpTargetParamsOIDLen-1] = SNMPTARGETPARAMSSECURITYLEVELCOLUMN;
  if ((temp_struct = search_snmpTargetParamsTable(snmpTargetParamsOID, snmpTargetParamsOIDLen, 
				     name, &name_len, 1)) == 0 ) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityLevel : BAD OID!\n"));
    return SNMP_ERR_NOSUCHNAME;
  }
  /* row exists, check if it is changeable */
  if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityLevel : row is read only\n"));
    return SNMP_ERR_READONLY;
  }
  /* check if row active */
  if (temp_struct->rowStatus == SNMP_ROW_ACTIVE) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsSecurityLevel : This change not allowed in active row.\n"));
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Finally, we're golden, check if we should save value */
  if (action == COMMIT) {
    temp_struct->secLevel = long_ret;
    
    /* If row is new, check if its status can be updated */
    if ( (temp_struct->rowStatus == SNMP_ROW_NOTREADY) &&
	 (snmpTargetParams_rowStatusCheck(temp_struct) != 0) )
      temp_struct->rowStatus = SNMP_ROW_NOTINSERVICE;
  }
  return SNMP_ERR_NOERROR;
} /* write_snmpTargetParamsSecurityLevel */


/* Assign a value to the Storage Type variable */
int
write_snmpTargetParamsStorageType(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  static long                    long_ret;
  size_t                         size;
  size_t                         bigsize=1000;
  struct targetParamTable_struct *temp_struct;

  if (var_val_type != ASN_INTEGER) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsStorageType not ASN_INTEGER\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsStorageType: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);
  
  if ( (long_ret != SNMP_STORAGE_OTHER) && (long_ret != SNMP_STORAGE_VOLATILE) &&
       (long_ret != SNMP_STORAGE_NONVOLATILE) )  {
    DEBUGMSGTL(("snmpTargetParamsEntry", "write to snmpTargetParamsStorageType : attempted storage type not a valid"));
    DEBUGMSG(("snmpTargetParamsEntry", "  value of other(%d), volatile(%d), or nonvolatile(%d)\n", 
	   SNMP_STORAGE_OTHER, SNMP_STORAGE_VOLATILE, SNMP_STORAGE_NONVOLATILE));
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Find the struct in the linked list and check status */
  snmpTargetParamsOID[snmpTargetParamsOIDLen-1] = SNMPTARGETPARAMSSTORAGETYPECOLUMN;
  if ((temp_struct = search_snmpTargetParamsTable(snmpTargetParamsOID, snmpTargetParamsOIDLen, 
				     name, &name_len, 1)) == 0 ) {
    DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamStorageType : BAD OID\n"));
    return SNMP_ERR_NOSUCHNAME;
  }
  if ( (temp_struct->storageType == SNMP_STORAGE_PERMANENT) || 
       (temp_struct->storageType == SNMP_STORAGE_READONLY) )  {
    DEBUGMSGTL(("snmpTargetParamsEntry", "write to snmpTargetParamsStorageType : row has unchangeable storage status: %d\n",
	   temp_struct->storageType));
    return SNMP_ERR_INCONSISTENTVALUE;
  }

  /* Finally, we're golden, check if we should save new value */
  if (action == COMMIT) {      
    temp_struct->storageType = long_ret;
  }

  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetParamsStorageType */


/* snmpTargeParams_createNewRow is called from write_snmpTargetParamsRowStatus
   when a new row is required. It creates a new row with 
   the index of the passed in 'name' (i.e. full index OID) and
   adds it to the linked list. 'name' should be the full OID of the new index. 
   It passes back 0 if unsuccessfull.*/
int snmpTargetParams_createNewRow(
     oid  *name,
     size_t name_len)
{
  int    pNameLen, i;
  struct targetParamTable_struct *temp_struct;

  /* setup a new snmpTargetParamTable structure and add it to the list */
  pNameLen = name_len - snmpTargetParamsOIDLen;
  if (pNameLen > 0) {
    temp_struct            = snmpTargetParamTable_create();
    temp_struct->paramName = (char *)malloc(sizeof(char)*(pNameLen + 1));

    for (i = 0; i < pNameLen; i++) {
      temp_struct->paramName[i] = (char)name[i+snmpTargetParamsOIDLen];
    }

    temp_struct->paramName[pNameLen]  = '\0';
    temp_struct->rowStatus            = SNMP_ROW_NOTREADY;
    
    snmpTargetParamTable_addToList(temp_struct, &aPTable);

    return 1;
  }

  return 0;
}  /* snmpTargetParams_createNewRow */
	

/* Assign a value to the Row Status variable */
int
write_snmpTargetParamsRowStatus(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  enum commit_action_enum        {NOTHING, DESTROY, CREATE, CHANGE};
  enum commit_action_enum        onCommitDo = NOTHING; 
  static long                    long_ret;
  size_t                         size;
  size_t                         bigsize=1000;
  struct targetParamTable_struct *temp_struct; /*also treated as boolean for row lookup*/

  if (var_val_type != ASN_INTEGER) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsRowStatus not ASN_INTEGER\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > (size = sizeof(long_ret))) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamsRowStatus: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  size = sizeof(long_ret);
  asn_parse_int(var_val, &bigsize, &var_val_type, &long_ret, size);

  /* search for struct in linked list */
  snmpTargetParamsOID[snmpTargetParamsOIDLen-1] = SNMPTARGETPARAMSROWSTATUSCOLUMN;
  if ((temp_struct = search_snmpTargetParamsTable(snmpTargetParamsOID, snmpTargetParamsOIDLen, 
				     name, &name_len, 1)) == 0) {
    /* row doesn't exist, check valid possibilities */
    if (long_ret == SNMP_ROW_DESTROY)  
      /* re: RFC 1903, destroying a non-existent row is noError, whatever */
      onCommitDo = NOTHING;
    /* check if this is for a new row creation */
    else if (long_ret == SNMP_ROW_CREATEANDGO || long_ret == SNMP_ROW_CREATEANDWAIT) 
      onCommitDo = CREATE;
    else /* no valid sets for no row being found so... */
      return SNMP_ERR_NOSUCHNAME;
  }
  else {  /* row exists */
    /* check if it is changeable */
    if (temp_struct->storageType == SNMP_STORAGE_READONLY) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamRowStatus : row is read only\n"));
      return SNMP_ERR_READONLY;
    }
    /* check if row is to be destroyed (note: it is ok to destroy notReady row!) */
    else if (long_ret == SNMP_ROW_DESTROY)  {
      if (temp_struct->storageType == SNMP_STORAGE_PERMANENT) {
	DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamRowStatus : unable to destroy permanent row\n"));
	return SNMP_ERR_INCONSISTENTVALUE;
      }
      else  {
	onCommitDo = DESTROY;
      }
    }
    /* check if row is new and can be changed from notready yet */
    else if (temp_struct->rowStatus == SNMP_ROW_NOTREADY) {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargeParamRowStatus : unable to change from NOTREADY\n"));
      return SNMP_ERR_INCONSISTENTVALUE;
    }  
    /* we now know the row status can be set, check for two valid settings left*/
    else if ( (long_ret == SNMP_ROW_ACTIVE) || 
	      (long_ret == SNMP_ROW_NOTINSERVICE) ) {
      onCommitDo = CHANGE;
    }
    /* not a valid setting */
    else  {
      DEBUGMSGTL(("snmpTargetParamsEntry","write to snmpTargetParamRowStatus : Bad value for set\n"));
      return SNMP_ERR_INCONSISTENTVALUE;
    }
  } /* if row exist */
  
  /* if this is a commit, do expected action */
  if (action == COMMIT) {
    switch(onCommitDo) { 
      
    case CREATE :
      if (snmpTargetParams_createNewRow(name, name_len) == 0) {
	DEBUGMSGTL(("snmpTargetParamsEntry", "write to snmpTargetParamsRowStatus : "));
	DEBUGMSG(("snmpTargetParamsEntry","failed new row creation, bad OID/index value \n"));
	return SNMP_ERR_GENERR;
      }
      break;
      
    case DESTROY:
      snmpTargetParamTable_remFromList(temp_struct, &aPTable);
      break;

    case CHANGE:
      temp_struct->rowStatus = long_ret;
      break;

    case NOTHING:
      break;
    }
  }
  
  return SNMP_ERR_NOERROR;
}  /* write_snmpTargetParamsRowStatus */








