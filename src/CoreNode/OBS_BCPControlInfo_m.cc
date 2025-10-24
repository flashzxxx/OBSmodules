//
// Generated file, do not edit! Created by nedtool 4.6 from src/CoreNode/OBS_BCPControlInfo.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "OBS_BCPControlInfo_m.h"

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

Register_Class(OBS_BCPControlInfo);

OBS_BCPControlInfo::OBS_BCPControlInfo() : ::cObject()
{
    this->port_var = -1;
    this->BCPArrival_var = -1;
}

OBS_BCPControlInfo::OBS_BCPControlInfo(const OBS_BCPControlInfo& other) : ::cObject(other)
{
    copy(other);
}

OBS_BCPControlInfo::~OBS_BCPControlInfo()
{
}

OBS_BCPControlInfo& OBS_BCPControlInfo::operator=(const OBS_BCPControlInfo& other)
{
    if (this==&other) return *this;
    ::cObject::operator=(other);
    copy(other);
    return *this;
}

void OBS_BCPControlInfo::copy(const OBS_BCPControlInfo& other)
{
    this->port_var = other.port_var;
    this->BCPArrival_var = other.BCPArrival_var;
}

void OBS_BCPControlInfo::parsimPack(cCommBuffer *b)
{
    doPacking(b,this->port_var);
    doPacking(b,this->BCPArrival_var);
}

void OBS_BCPControlInfo::parsimUnpack(cCommBuffer *b)
{
    doUnpacking(b,this->port_var);
    doUnpacking(b,this->BCPArrival_var);
}

int OBS_BCPControlInfo::getPort() const
{
    return port_var;
}

void OBS_BCPControlInfo::setPort(int port)
{
    this->port_var = port;
}

simtime_t OBS_BCPControlInfo::getBCPArrival() const
{
    return BCPArrival_var;
}

void OBS_BCPControlInfo::setBCPArrival(simtime_t BCPArrival)
{
    this->BCPArrival_var = BCPArrival;
}

class OBS_BCPControlInfoDescriptor : public cClassDescriptor
{
  public:
    OBS_BCPControlInfoDescriptor();
    virtual ~OBS_BCPControlInfoDescriptor();

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

Register_ClassDescriptor(OBS_BCPControlInfoDescriptor);

OBS_BCPControlInfoDescriptor::OBS_BCPControlInfoDescriptor() : cClassDescriptor("OBS_BCPControlInfo", "cObject")
{
}

OBS_BCPControlInfoDescriptor::~OBS_BCPControlInfoDescriptor()
{
}

bool OBS_BCPControlInfoDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<OBS_BCPControlInfo *>(obj)!=NULL;
}

const char *OBS_BCPControlInfoDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int OBS_BCPControlInfoDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int OBS_BCPControlInfoDescriptor::getFieldTypeFlags(void *object, int field) const
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
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *OBS_BCPControlInfoDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "port",
        "BCPArrival",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int OBS_BCPControlInfoDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='p' && strcmp(fieldName, "port")==0) return base+0;
    if (fieldName[0]=='B' && strcmp(fieldName, "BCPArrival")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *OBS_BCPControlInfoDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "simtime_t",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *OBS_BCPControlInfoDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int OBS_BCPControlInfoDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BCPControlInfo *pp = (OBS_BCPControlInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string OBS_BCPControlInfoDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BCPControlInfo *pp = (OBS_BCPControlInfo *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getPort());
        case 1: return double2string(pp->getBCPArrival());
        default: return "";
    }
}

bool OBS_BCPControlInfoDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BCPControlInfo *pp = (OBS_BCPControlInfo *)object; (void)pp;
    switch (field) {
        case 0: pp->setPort(string2long(value)); return true;
        case 1: pp->setBCPArrival(string2double(value)); return true;
        default: return false;
    }
}

const char *OBS_BCPControlInfoDescriptor::getFieldStructName(void *object, int field) const
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

void *OBS_BCPControlInfoDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_BCPControlInfo *pp = (OBS_BCPControlInfo *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


