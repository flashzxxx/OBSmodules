//
// Generated file, do not edit! Created by nedtool 4.6 from src/EdgeNode/OBS_BurstSenderInfo.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "OBS_BurstSenderInfo_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

Register_Class(OBS_BurstSenderInfo);

OBS_BurstSenderInfo::OBS_BurstSenderInfo() : ::cObject()
{
    this->burstId_var = -1;
    this->burstifierId_var = -1;
    this->numSeq_var = -1;
    this->assignedLambda_var = -1;
    this->label_var = -1;
}

OBS_BurstSenderInfo::OBS_BurstSenderInfo(const OBS_BurstSenderInfo& other) : ::cObject(other)
{
    copy(other);
}

OBS_BurstSenderInfo::~OBS_BurstSenderInfo()
{
}

OBS_BurstSenderInfo& OBS_BurstSenderInfo::operator=(const OBS_BurstSenderInfo& other)
{
    if (this==&other) return *this;
    ::cObject::operator=(other);
    copy(other);
    return *this;
}

void OBS_BurstSenderInfo::copy(const OBS_BurstSenderInfo& other)
{
    this->burstId_var = other.burstId_var;
    this->burstifierId_var = other.burstifierId_var;
    this->numSeq_var = other.numSeq_var;
    this->assignedLambda_var = other.assignedLambda_var;
    this->label_var = other.label_var;
}

void OBS_BurstSenderInfo::parsimPack(cCommBuffer *b)
{
    doPacking(b,this->burstId_var);
    doPacking(b,this->burstifierId_var);
    doPacking(b,this->numSeq_var);
    doPacking(b,this->assignedLambda_var);
    doPacking(b,this->label_var);
}

void OBS_BurstSenderInfo::parsimUnpack(cCommBuffer *b)
{
    doUnpacking(b,this->burstId_var);
    doUnpacking(b,this->burstifierId_var);
    doUnpacking(b,this->numSeq_var);
    doUnpacking(b,this->assignedLambda_var);
    doUnpacking(b,this->label_var);
}

int OBS_BurstSenderInfo::getBurstId() const
{
    return burstId_var;
}

void OBS_BurstSenderInfo::setBurstId(int burstId)
{
    this->burstId_var = burstId;
}

int OBS_BurstSenderInfo::getBurstifierId() const
{
    return burstifierId_var;
}

void OBS_BurstSenderInfo::setBurstifierId(int burstifierId)
{
    this->burstifierId_var = burstifierId;
}

int OBS_BurstSenderInfo::getNumSeq() const
{
    return numSeq_var;
}

void OBS_BurstSenderInfo::setNumSeq(int numSeq)
{
    this->numSeq_var = numSeq;
}

int OBS_BurstSenderInfo::getAssignedLambda() const
{
    return assignedLambda_var;
}

void OBS_BurstSenderInfo::setAssignedLambda(int assignedLambda)
{
    this->assignedLambda_var = assignedLambda;
}

int OBS_BurstSenderInfo::getLabel() const
{
    return label_var;
}

void OBS_BurstSenderInfo::setLabel(int label)
{
    this->label_var = label;
}

class OBS_BurstSenderInfoDescriptor : public cClassDescriptor
{
  public:
    OBS_BurstSenderInfoDescriptor();
    virtual ~OBS_BurstSenderInfoDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(OBS_BurstSenderInfoDescriptor);

OBS_BurstSenderInfoDescriptor::OBS_BurstSenderInfoDescriptor() : cClassDescriptor("OBS_BurstSenderInfo", "cObject")
{
}

OBS_BurstSenderInfoDescriptor::~OBS_BurstSenderInfoDescriptor()
{
}

bool OBS_BurstSenderInfoDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<OBS_BurstSenderInfo *>(obj)!=NULL;
}

const char *OBS_BurstSenderInfoDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int OBS_BurstSenderInfoDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount(object) : 5;
}

unsigned int OBS_BurstSenderInfoDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *OBS_BurstSenderInfoDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "burstId",
        "burstifierId",
        "numSeq",
        "assignedLambda",
        "label",
    };
    return (field>=0 && field<5) ? fieldNames[field] : NULL;
}

int OBS_BurstSenderInfoDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='b' && strcmp(fieldName, "burstId")==0) return base+0;
    if (fieldName[0]=='b' && strcmp(fieldName, "burstifierId")==0) return base+1;
    if (fieldName[0]=='n' && strcmp(fieldName, "numSeq")==0) return base+2;
    if (fieldName[0]=='a' && strcmp(fieldName, "assignedLambda")==0) return base+3;
    if (fieldName[0]=='l' && strcmp(fieldName, "label")==0) return base+4;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *OBS_BurstSenderInfoDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "int",
        "int",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : NULL;
}

const char *OBS_BurstSenderInfoDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int OBS_BurstSenderInfoDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BurstSenderInfo *pp = (OBS_BurstSenderInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string OBS_BurstSenderInfoDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BurstSenderInfo *pp = (OBS_BurstSenderInfo *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getBurstId());
        case 1: return long2string(pp->getBurstifierId());
        case 2: return long2string(pp->getNumSeq());
        case 3: return long2string(pp->getAssignedLambda());
        case 4: return long2string(pp->getLabel());
        default: return "";
    }
}

bool OBS_BurstSenderInfoDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BurstSenderInfo *pp = (OBS_BurstSenderInfo *)object; (void)pp;
    switch (field) {
        case 0: pp->setBurstId(string2long(value)); return true;
        case 1: pp->setBurstifierId(string2long(value)); return true;
        case 2: pp->setNumSeq(string2long(value)); return true;
        case 3: pp->setAssignedLambda(string2long(value)); return true;
        case 4: pp->setLabel(string2long(value)); return true;
        default: return false;
    }
}

const char *OBS_BurstSenderInfoDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    };
}

void *OBS_BurstSenderInfoDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BurstSenderInfo *pp = (OBS_BurstSenderInfo *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


