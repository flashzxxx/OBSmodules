//
// Generated file, do not edit! Created by nedtool 4.6 from src/CoreNode/OBS_CoreRoutingTableEntry.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "OBS_CoreRoutingTableEntry_m.h"

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

Register_Class(OBS_CoreRoutingTableEntry);

OBS_CoreRoutingTableEntry::OBS_CoreRoutingTableEntry() : ::cObject()
{
    this->inPort_var = -1;
    this->inColour_var = -1;
    this->inLabel_var = -1;
    this->outPort_var = -1;
    this->outColour_var = -1;
    this->outLabel_var = -1;
}

OBS_CoreRoutingTableEntry::OBS_CoreRoutingTableEntry(const OBS_CoreRoutingTableEntry& other) : ::cObject(other)
{
    copy(other);
}

OBS_CoreRoutingTableEntry::~OBS_CoreRoutingTableEntry()
{
}

OBS_CoreRoutingTableEntry& OBS_CoreRoutingTableEntry::operator=(const OBS_CoreRoutingTableEntry& other)
{
    if (this==&other) return *this;
    ::cObject::operator=(other);
    copy(other);
    return *this;
}

void OBS_CoreRoutingTableEntry::copy(const OBS_CoreRoutingTableEntry& other)
{
    this->inPort_var = other.inPort_var;
    this->inColour_var = other.inColour_var;
    this->inLabel_var = other.inLabel_var;
    this->outPort_var = other.outPort_var;
    this->outColour_var = other.outColour_var;
    this->outLabel_var = other.outLabel_var;
}

void OBS_CoreRoutingTableEntry::parsimPack(cCommBuffer *b)
{
    doPacking(b,this->inPort_var);
    doPacking(b,this->inColour_var);
    doPacking(b,this->inLabel_var);
    doPacking(b,this->outPort_var);
    doPacking(b,this->outColour_var);
    doPacking(b,this->outLabel_var);
}

void OBS_CoreRoutingTableEntry::parsimUnpack(cCommBuffer *b)
{
    doUnpacking(b,this->inPort_var);
    doUnpacking(b,this->inColour_var);
    doUnpacking(b,this->inLabel_var);
    doUnpacking(b,this->outPort_var);
    doUnpacking(b,this->outColour_var);
    doUnpacking(b,this->outLabel_var);
}

int OBS_CoreRoutingTableEntry::getInPort() const
{
    return inPort_var;
}

void OBS_CoreRoutingTableEntry::setInPort(int inPort)
{
    this->inPort_var = inPort;
}

int OBS_CoreRoutingTableEntry::getInColour() const
{
    return inColour_var;
}

void OBS_CoreRoutingTableEntry::setInColour(int inColour)
{
    this->inColour_var = inColour;
}

int OBS_CoreRoutingTableEntry::getInLabel() const
{
    return inLabel_var;
}

void OBS_CoreRoutingTableEntry::setInLabel(int inLabel)
{
    this->inLabel_var = inLabel;
}

int OBS_CoreRoutingTableEntry::getOutPort() const
{
    return outPort_var;
}

void OBS_CoreRoutingTableEntry::setOutPort(int outPort)
{
    this->outPort_var = outPort;
}

int OBS_CoreRoutingTableEntry::getOutColour() const
{
    return outColour_var;
}

void OBS_CoreRoutingTableEntry::setOutColour(int outColour)
{
    this->outColour_var = outColour;
}

int OBS_CoreRoutingTableEntry::getOutLabel() const
{
    return outLabel_var;
}

void OBS_CoreRoutingTableEntry::setOutLabel(int outLabel)
{
    this->outLabel_var = outLabel;
}

class OBS_CoreRoutingTableEntryDescriptor : public cClassDescriptor
{
  public:
    OBS_CoreRoutingTableEntryDescriptor();
    virtual ~OBS_CoreRoutingTableEntryDescriptor();

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

Register_ClassDescriptor(OBS_CoreRoutingTableEntryDescriptor);

OBS_CoreRoutingTableEntryDescriptor::OBS_CoreRoutingTableEntryDescriptor() : cClassDescriptor("OBS_CoreRoutingTableEntry", "cObject")
{
}

OBS_CoreRoutingTableEntryDescriptor::~OBS_CoreRoutingTableEntryDescriptor()
{
}

bool OBS_CoreRoutingTableEntryDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<OBS_CoreRoutingTableEntry *>(obj)!=NULL;
}

const char *OBS_CoreRoutingTableEntryDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int OBS_CoreRoutingTableEntryDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount(object) : 6;
}

unsigned int OBS_CoreRoutingTableEntryDescriptor::getFieldTypeFlags(void *object, int field) const
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
        FD_ISEDITABLE,
    };
    return (field>=0 && field<6) ? fieldTypeFlags[field] : 0;
}

const char *OBS_CoreRoutingTableEntryDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "inPort",
        "inColour",
        "inLabel",
        "outPort",
        "outColour",
        "outLabel",
    };
    return (field>=0 && field<6) ? fieldNames[field] : NULL;
}

int OBS_CoreRoutingTableEntryDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='i' && strcmp(fieldName, "inPort")==0) return base+0;
    if (fieldName[0]=='i' && strcmp(fieldName, "inColour")==0) return base+1;
    if (fieldName[0]=='i' && strcmp(fieldName, "inLabel")==0) return base+2;
    if (fieldName[0]=='o' && strcmp(fieldName, "outPort")==0) return base+3;
    if (fieldName[0]=='o' && strcmp(fieldName, "outColour")==0) return base+4;
    if (fieldName[0]=='o' && strcmp(fieldName, "outLabel")==0) return base+5;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *OBS_CoreRoutingTableEntryDescriptor::getFieldTypeString(void *object, int field) const
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
        "int",
    };
    return (field>=0 && field<6) ? fieldTypeStrings[field] : NULL;
}

const char *OBS_CoreRoutingTableEntryDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int OBS_CoreRoutingTableEntryDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    OBS_CoreRoutingTableEntry *pp = (OBS_CoreRoutingTableEntry *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string OBS_CoreRoutingTableEntryDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_CoreRoutingTableEntry *pp = (OBS_CoreRoutingTableEntry *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getInPort());
        case 1: return long2string(pp->getInColour());
        case 2: return long2string(pp->getInLabel());
        case 3: return long2string(pp->getOutPort());
        case 4: return long2string(pp->getOutColour());
        case 5: return long2string(pp->getOutLabel());
        default: return "";
    }
}

bool OBS_CoreRoutingTableEntryDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    OBS_CoreRoutingTableEntry *pp = (OBS_CoreRoutingTableEntry *)object; (void)pp;
    switch (field) {
        case 0: pp->setInPort(string2long(value)); return true;
        case 1: pp->setInColour(string2long(value)); return true;
        case 2: pp->setInLabel(string2long(value)); return true;
        case 3: pp->setOutPort(string2long(value)); return true;
        case 4: pp->setOutColour(string2long(value)); return true;
        case 5: pp->setOutLabel(string2long(value)); return true;
        default: return false;
    }
}

const char *OBS_CoreRoutingTableEntryDescriptor::getFieldStructName(void *object, int field) const
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

void *OBS_CoreRoutingTableEntryDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    OBS_CoreRoutingTableEntry *pp = (OBS_CoreRoutingTableEntry *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


