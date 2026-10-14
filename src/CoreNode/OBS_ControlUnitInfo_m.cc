//
// Generated file, do not edit! Created by nedtool 4.6 from src/CoreNode/OBS_ControlUnitInfo.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "OBS_ControlUnitInfo_m.h"

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

Register_Class(OBS_ControlUnitInfo);

OBS_ControlUnitInfo::OBS_ControlUnitInfo(const char *name, int kind) : ::cMessage(name,kind)
{
    this->inGate_var = -1;
    this->outGate_var = -1;
}

OBS_ControlUnitInfo::OBS_ControlUnitInfo(const OBS_ControlUnitInfo& other) : ::cMessage(other)
{
    copy(other);
}

OBS_ControlUnitInfo::~OBS_ControlUnitInfo()
{
}

OBS_ControlUnitInfo& OBS_ControlUnitInfo::operator=(const OBS_ControlUnitInfo& other)
{
    if (this==&other) return *this;
    ::cMessage::operator=(other);
    copy(other);
    return *this;
}

void OBS_ControlUnitInfo::copy(const OBS_ControlUnitInfo& other)
{
    this->inGate_var = other.inGate_var;
    this->outGate_var = other.outGate_var;
}

void OBS_ControlUnitInfo::parsimPack(cCommBuffer *b)
{
    ::cMessage::parsimPack(b);
    doPacking(b,this->inGate_var);
    doPacking(b,this->outGate_var);
}

void OBS_ControlUnitInfo::parsimUnpack(cCommBuffer *b)
{
    ::cMessage::parsimUnpack(b);
    doUnpacking(b,this->inGate_var);
    doUnpacking(b,this->outGate_var);
}

int OBS_ControlUnitInfo::getInGate() const
{
    return inGate_var;
}

void OBS_ControlUnitInfo::setInGate(int inGate)
{
    this->inGate_var = inGate;
}

int OBS_ControlUnitInfo::getOutGate() const
{
    return outGate_var;
}

void OBS_ControlUnitInfo::setOutGate(int outGate)
{
    this->outGate_var = outGate;
}

class OBS_ControlUnitInfoDescriptor : public cClassDescriptor
{
  public:
    OBS_ControlUnitInfoDescriptor();
    virtual ~OBS_ControlUnitInfoDescriptor();

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

Register_ClassDescriptor(OBS_ControlUnitInfoDescriptor);

OBS_ControlUnitInfoDescriptor::OBS_ControlUnitInfoDescriptor() : cClassDescriptor("OBS_ControlUnitInfo", "cMessage")
{
}

OBS_ControlUnitInfoDescriptor::~OBS_ControlUnitInfoDescriptor()
{
}

bool OBS_ControlUnitInfoDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<OBS_ControlUnitInfo *>(obj)!=NULL;
}

const char *OBS_ControlUnitInfoDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int OBS_ControlUnitInfoDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int OBS_ControlUnitInfoDescriptor::getFieldTypeFlags(void *object, int field) const
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

const char *OBS_ControlUnitInfoDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "inGate",
        "outGate",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int OBS_ControlUnitInfoDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='i' && strcmp(fieldName, "inGate")==0) return base+0;
    if (fieldName[0]=='o' && strcmp(fieldName, "outGate")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *OBS_ControlUnitInfoDescriptor::getFieldTypeString(void *object, int field) const
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
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *OBS_ControlUnitInfoDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int OBS_ControlUnitInfoDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    OBS_ControlUnitInfo *pp = (OBS_ControlUnitInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string OBS_ControlUnitInfoDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_ControlUnitInfo *pp = (OBS_ControlUnitInfo *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getInGate());
        case 1: return long2string(pp->getOutGate());
        default: return "";
    }
}

bool OBS_ControlUnitInfoDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    OBS_ControlUnitInfo *pp = (OBS_ControlUnitInfo *)object; (void)pp;
    switch (field) {
        case 0: pp->setInGate(string2long(value)); return true;
        case 1: pp->setOutGate(string2long(value)); return true;
        default: return false;
    }
}

const char *OBS_ControlUnitInfoDescriptor::getFieldStructName(void *object, int field) const
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

void *OBS_ControlUnitInfoDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_ControlUnitInfo *pp = (OBS_ControlUnitInfo *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


