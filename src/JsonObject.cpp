#include "JsonObject.h"
#include "Assertions.h"
#include <limits.h>
#include <float.h>

namespace Json
{
	const JsonObject& JsonObject::NullSingleton()
	{
		static const JsonObject nullStatic;
		return nullStatic;
	}

	JsonObject::JsonObject(ValueType type)
		: type(type)
	{
		switch (type)
		{
		case Json::ValueType::Null:
			_value.Map = nullptr;
			break;
		case Json::ValueType::String:
			_value.String = nullptr;
			break;
		case Json::ValueType::Bool:
			_value.Bool = false;
			break;
		case Json::ValueType::Int:
			_value.Int = 0;
			break;
		case Json::ValueType::UInt:
			_value.UInt = 0u;
			break;
		case Json::ValueType::Float:
			_value.Float = 0.0f;
			break;
		case Json::ValueType::Object:
			_value.Map = new unordered_map<string, JsonObject>;
			break;
		case Json::ValueType::Array:
			_value.Array = new vector<JsonObject>;
			break;
		default:
			_value.Map = nullptr;
			break;
		}
	}

	JsonObject::JsonObject(const char* value)
	{
		type = ValueType::String;
		_value.String = DublicateStringValue(value, strlen(value));
	}

	JsonObject::JsonObject(const string& value)
	{
		type = ValueType::String;
		_value.String = DublicateStringValue(value.data(), value.length());
	}

	JsonObject::JsonObject(bool value)
	{
		type = ValueType::Bool;
		_value.Bool = value;
	}

	JsonObject::JsonObject(int value)
	{
		type = ValueType::Int;
		_value.Int = value;
	}

	JsonObject::JsonObject(unsigned int value)
	{
		type = ValueType::UInt;
		_value.UInt = value;
	}

	JsonObject::JsonObject(float value)
	{
		type = ValueType::Float;
		_value.Float = value;
	}

	JsonObject::JsonObject(const JsonObject& other)
	{
		type = other.type;
		// If type is object/array do deep copy.
		if (type == ValueType::Object)
			_value.Map = new unordered_map<string, JsonObject>(*other._value.Map);
		else if (type == ValueType::Array)
			_value.Array = new vector<JsonObject>(*other._value.Array);
		else if (type == ValueType::String)
			_value.String = DublicateStringValue(other._value.String, strlen(other._value.String));
		else
			_value = other._value;
	}
	
	JsonObject::JsonObject(JsonObject&& other) noexcept
		: type(other.type), _value(std::move(other._value))
	{
		// Reset.
		other._value.Map = nullptr;
	}

	JsonObject::~JsonObject()
	{
		CleanUp();
	}

	JsonObject& JsonObject::operator=(JsonObject&& other) noexcept
	{
		// Cleanup.
		CleanUp();
		// Member-wise move.
		type = other.type;
		_value = std::move(other._value);
		// Reset.
		other._value.Map = nullptr;

		return *this;
	}

	bool JsonObject::operator==(const JsonObject& other) const
	{
		// If types are not matching, then immediatly return false.
		if (GetType() != other.GetType())
			return false;

		switch (GetType())
		{
		case Json::ValueType::Null:
			return true;
		case Json::ValueType::String:
		{
			unsigned int length = strlen(_value.String);
			unsigned int otherLength = strlen(other._value.String);
			if (length != otherLength)
				return false;

			int cmp = strcmp(_value.String, other._value.String);
			return cmp == 0;
		}
		case Json::ValueType::Bool:
			return _value.Bool == other._value.Bool;
		case Json::ValueType::Int:
			return _value.Int == other._value.Int;
			return _value.Int == other._value.Int;
		case Json::ValueType::UInt:
			return _value.UInt == other._value.UInt;
		case Json::ValueType::Float:
			return _value.Float == other._value.Float;
		case Json::ValueType::Object:
			if (_value.Map->size() != other._value.Map->size())
				return false;

			return (*_value.Map) == (*other._value.Map);
		case Json::ValueType::Array:
			if (_value.Array->size() != other._value.Array->size())
				return false;

			return (*_value.Array) == (*other._value.Array);
		default:
			return false; // unreachable.
		}
	}
	
	bool JsonObject::operator!=(const JsonObject& other) const
	{
		return !(*this == other);
	}

	const JsonObject& JsonObject::operator[](const char* key) const
	{
		string sKey = key;
		return (*this)[sKey];
	}

	const JsonObject& JsonObject::operator[](const string& key) const
	{
		ASSERT_TRUE((GetType() == ValueType::Object || GetType() == ValueType::Null),
			"in JsonObject::operator[] requires ObjectValue!");

		auto itr = _value.Map->find(key);
		if (itr == _value.Map->end())
			return NullSingleton();
		return itr->second;
	}

	JsonObject& JsonObject::operator[](const char* key)
	{
		string sKey = key;
		return (*this)[sKey];
	}

	JsonObject& JsonObject::operator[](const string& key)
	{
		ASSERT_TRUE((GetType() == ValueType::Object || GetType() == ValueType::Null),
			"in JsonObject::operator[] requires ObjectValue or NullValue!");

		// If NullValue, then make ObjectValue.
		if (GetType() == ValueType::Null)
			(*this) = JsonObject(ValueType::Object);

		auto itr = _value.Map->find(key);
		if (itr != _value.Map->end()) // If exists.
			return itr->second;

		std::pair<string, JsonObject> defaultValue(key, JsonObject());
		itr = _value.Map->insert(itr, std::move(defaultValue));
		return itr->second;
	}

	const JsonObject& JsonObject::operator[](int index) const
	{
		ASSERT_TRUE((index >= 0 && index < Size()),
			"in JsonObject::operator[](int index): index out of range.");
		ASSERT_TRUE((GetType() == ValueType::Array),
			"in JsonObject::operator[](int index): requires ArrayValue!");

		return _value.Array->at(index);
	}

	JsonObject& JsonObject::operator[](int index)
	{
		ASSERT_TRUE((index >= 0 && index < Size()),
			"in JsonObject::operator[](int index): index out of range.");
		ASSERT_TRUE((GetType() == ValueType::Array),
			"in JsonObject::operator[](int index): requires ArrayValue!");

		return _value.Array->at(index);
	}
	
	JsonObject::ObjectIterator JsonObject::ObjectBegin()
	{
		if (GetType() != ValueType::Object)
			return ObjectIterator();
		return _value.Map->begin();
	}

	JsonObject::ObjectConstIterator JsonObject::ObjectConstBegin() const
	{
		if (GetType() != ValueType::Object)
			return ObjectConstIterator();
		return _value.Map->cbegin();
	}

	JsonObject::ObjectIterator JsonObject::ObjectEnd()
	{
		if (GetType() != ValueType::Object)
			return ObjectIterator();
		return _value.Map->end();
	}

	JsonObject::ObjectConstIterator JsonObject::ObjectConstEnd() const
	{
		if (GetType() != ValueType::Object)
			return ObjectConstIterator();
		return _value.Map->cend();
	}

	JsonObject::ArrayIterator JsonObject::ArrayBegin()
	{
		if (GetType() != ValueType::Array)
			return ArrayIterator();
		return _value.Array->begin();
	}

	JsonObject::ArrayConstIterator JsonObject::ArrayConstBegin() const
	{
		if (GetType() != ValueType::Array)
			return ArrayConstIterator();
		return _value.Array->cbegin();
	}

	JsonObject::ArrayIterator JsonObject::ArrayEnd()
	{
		if (GetType() != ValueType::Array)
			return ArrayIterator();
		return _value.Array->end();
	}

	JsonObject::ArrayConstIterator JsonObject::ArrayConstEnd() const
	{
		if (GetType() != ValueType::Array)
			return ArrayConstIterator();
		return _value.Array->cend();
	}

	void JsonObject::Append(const JsonObject& object)
	{
		return Append(JsonObject(object));
	}
	
	void JsonObject::Append(JsonObject&& object)
	{
		ASSERT_TRUE((GetType() == ValueType::Array || GetType() == ValueType::Null),
			"in JsonObject::Append: requires Array Value!");

		// If this type is Null, then construct it as array type.
		if (GetType() == ValueType::Null)
			*this = JsonObject(ValueType::Array);

		_value.Array->emplace_back(std::move(object));
	}

	int JsonObject::Size() const
	{
		if (GetType() == ValueType::Object)
			return _value.Map->size();
		else if (GetType() == ValueType::Array)
			return _value.Array->size();
		return 0;
	}
	
	void JsonObject::Clear()
	{
		switch (GetType())
		{
		case ValueType::Object:
			_value.Map->clear();
			break;
		case ValueType::Array:
			_value.Array->clear();
			break;
		case ValueType::Null: break;
		default:
			ASSERT_MESSAGE("in JsonObject::Clear: requires Complex Value!");
			break;
		}
	}
	
	void JsonObject::Resize(unsigned int newSize)
	{
		ASSERT_TRUE((GetType() == ValueType::Null || GetType() == ValueType::Array),
			"in JsonObject::Resize: requires Array Value!");

		// If NullValue, then make ArrayValue.
		if (GetType() == ValueType::Null)
			(*this) = JsonObject(ValueType::Array);

		unsigned int oldSize = Size();
		if (oldSize == newSize)
			return;
		else if (newSize == 0)
			Clear();
		else if (oldSize < newSize) // New size is greater, add items.
		{
			for (size_t i = oldSize; i < newSize; i++)
				_value.Array->emplace_back(JsonObject(ValueType::Null));
		}
		else if (oldSize > newSize) // Old size is greater, remove items.
		{
			for (size_t i = oldSize; i > newSize; i--)
				_value.Array->pop_back();
		}
	}

	bool JsonObject::IsValidIndex(int index) const
	{
		ASSERT_TRUE((GetType() == ValueType::Object || GetType() == ValueType::Array || GetType() == ValueType::Null),
			"in JsonObject::IsValidIndex: requires Object or Array Value!");

		return index < Size();
	}

	bool JsonObject::Insert(int index, const JsonObject& object)
	{
		return Insert(index, JsonObject(object));
	}
	
	bool JsonObject::Insert(int index, JsonObject&& object)
	{
		ASSERT_TRUE((GetType() == ValueType::Array || GetType() == ValueType::Null),
			"in JsonObject::Insert: requires Array Value!");

		if (index > Size())
			return false;

		if (GetType() == ValueType::Null)
			Append(std::move(object));
		else // Array
		{
			auto itr = _value.Array->cbegin() + index;
			_value.Array->insert(itr, std::move(object));
		}
		return true;
	}
	
	bool JsonObject::RemoveMember(const char* key, JsonObject* removed)
	{
		string sKey = key;
		return RemoveMember(sKey, removed);
	}
	
	bool JsonObject::RemoveMember(const string& key, JsonObject* removed)
	{
		ASSERT_TRUE((GetType() == ValueType::Null || GetType() == ValueType::Object),
			"in JsonObject::RemoveMember: requires Object Value!");

		if (GetType() == ValueType::Null)
			return false;

		auto itr = _value.Map->find(key);
		if (itr == _value.Map->end()) // If not found.
			return false;

		removed = &(itr->second);
		_value.Map->erase(itr);
		return true;
	}

	bool JsonObject::RemoveIndex(int index, JsonObject* removed)
	{
		ASSERT_TRUE((GetType() == ValueType::Null || GetType() == ValueType::Array),
			"in JsonObject::RemoveIndex: requires Array Value!");

		if (GetType() == ValueType::Null)
			return false;
		if (!IsValidIndex(index))
			return false;

		auto itr = _value.Array->cbegin() + index;
		if (itr == _value.Array->end())
			return false;
		_value.Array->erase(itr);
		return true;
	}

	bool JsonObject::IsMember(const char* key) const
	{
		string sKey = key;
		return IsMember(sKey);
	}

	bool JsonObject::IsMember(const string& key) const
	{
		ASSERT_TRUE((GetType() == ValueType::Null || GetType() == ValueType::Object),
			"in JsonObject::IsMember: requires Object Value!");

		if (GetType() == ValueType::Null)
			return false;

		auto itr = _value.Map->find(key);
		if (itr == _value.Map->end()) // If not found.
			return false;
		return true;
	}

	vector<string> JsonObject::GetMemberNames() const
	{
		ASSERT_TRUE((GetType() == ValueType::Null || GetType() == ValueType::Object),
			"in JsonObject::GetMemberNames: requires Object Value!");

		// If Null return empty vector.
		if (GetType() == ValueType::Null)
			return vector<string>();

		vector<string> names;
		for (auto itr = _value.Map->cbegin(); itr != _value.Map->cend(); itr++)
			names.push_back(itr->first);

		return names;
	}

	bool JsonObject::IsIntegral(float value) const
	{
		float integralPart;
		return modf(value, &integralPart) == 0.0f;
	}

	bool JsonObject::IsInt() const
	{
		switch (GetType())
		{
		case Json::ValueType::Int:
			return true;
		case Json::ValueType::UInt:
			return _value.UInt <= INT_MAX;
		case Json::ValueType::Float:
			return _value.Float >= INT_MIN && _value.Float <= INT_MAX && IsIntegral(_value.Float);
		default:
			break;
		}
		return false;
	}

	bool JsonObject::IsUInt() const
	{
		switch (GetType())
		{
		case ValueType::UInt:
			return true;
		case ValueType::Int:
			return _value.UInt >= 0;
		case ValueType::Float:
			return _value.Float >= 0.0f && _value.Float <= UINT_MAX && IsIntegral(_value.Float);
		default:
			break;
		}
		return false;
	}

	bool JsonObject::IsFloat() const
	{
		switch (GetType())
		{
		case ValueType::Float:
			return true;
		case ValueType::Int:
			return _value.Int >= -FLT_MAX && _value.Int <= FLT_MAX;
		case ValueType::UInt:
			return _value.UInt <= FLT_MAX;
		default:
			break;
		}
		return false;
	}

	bool JsonObject::IsNumeric() const
	{
		return GetType() == ValueType::Int || GetType() == ValueType::UInt || GetType() == ValueType::Float;
	}
	
	bool JsonObject::IsConvertibleTo(ValueType otherType) const
	{
		switch (otherType)
		{
		case ValueType::Null:
			return (IsNumeric() && _value.Float == 0.0f) ||
				(GetType() == ValueType::Bool && !_value.Bool) ||
				(GetType() == ValueType::String && AsString().empty()) ||
				(GetType() == ValueType::Object && _value.Map->empty()) ||
				(GetType() == ValueType::Array && _value.Array->empty()) ||
				(GetType() == ValueType::Null);
		case ValueType::String:
			return IsNumeric() ||
				(GetType() == ValueType::Bool) ||
				(GetType() == ValueType::String) ||
				(GetType() == ValueType::Null);
		case ValueType::Bool:
			return IsNumeric() ||
				(GetType() == ValueType::Bool) ||
				(GetType() == ValueType::Null);
		case ValueType::Int:
			return IsInt() ||
				(GetType() == ValueType::UInt && _value.UInt <= INT_MAX) ||
				(GetType() == ValueType::Float && _value.Float >= INT_MIN && _value.Float <= INT_MAX) ||
				(GetType() == ValueType::Bool) ||
				(GetType() == ValueType::Null);
		case ValueType::UInt:
			return IsUInt() ||
				(GetType() == ValueType::Int && _value.Int >= 0) ||
				(GetType() == ValueType::Float && _value.Float >= 0.0f && _value.Float <= UINT_MAX) ||
				(GetType() == ValueType::Bool) ||
				(GetType() == ValueType::Null);
		case ValueType::Float:
			return IsNumeric() ||
				(GetType() == ValueType::Bool) ||
				(GetType() == ValueType::Null);
		case ValueType::Object:
			return (GetType() == ValueType::Object) || (GetType() == ValueType::Null);
		case ValueType::Array:
			return (GetType() == ValueType::Array) || (GetType() == ValueType::Null);
		default:
			return false;
		}
	}

	string JsonObject::AsString() const
	{
		string value;
		bool succeed = AsString(value);
		ASSERT_TRUE(succeed, "in JsonObject::AsString: Type is not convertible to string.");
		return value;
	}

	bool JsonObject::AsBool() const
	{
		bool value;
		bool succeed = AsBool(value);
		ASSERT_TRUE(succeed, "in JsonObject::AsBool: Type is not convertible to bool.");
		return value;
	}

	int JsonObject::AsInt() const
	{
		int value;
		bool succeed = AsInt(value);
		ASSERT_TRUE(succeed, "in JsonObject::AsInt: Type is not convertible to int.");
		return value;
	}

	unsigned int JsonObject::AsUInt() const
	{
		unsigned value;
		bool succeed = AsUInt(value);
		ASSERT_TRUE(succeed, "in JsonObject::AsUInt: Type is not convertible to unsigned int.");
		return value;
	}

	float JsonObject::AsFloat() const
	{
		float value;
		bool succeed = AsFloat(value);
		ASSERT_TRUE(succeed, "in JsonObject::AsFloat: Type is not convertible to float.");
		return value;
	}

	bool JsonObject::AsString(string& value) const
	{
		switch (GetType())
		{
		case Json::ValueType::Null:
			value = "null";
			break;
		case Json::ValueType::String:
			if (_value.String == nullptr)
				value = "";
			else
				value = _value.String;
			break;
		case Json::ValueType::Bool:
			value = _value.Bool ? "true" : "false";
			break;
		case Json::ValueType::Int:
			value = std::to_string(_value.Int);
			break;
		case Json::ValueType::UInt:
			value = std::to_string(_value.UInt);
			break;
		case Json::ValueType::Float:
			value = std::to_string(_value.Float);
			break;
		case Json::ValueType::Object:
		case Json::ValueType::Array:
		default:
			return false;
		}
		return true;
	}

	bool JsonObject::AsBool(bool& value) const
	{
		switch (GetType())
		{
		case Json::ValueType::Null:
			value = false;
			break;
		case Json::ValueType::Bool:
			value = _value.Bool;
			break;
		case Json::ValueType::Int:
			value = _value.Int;
			break;
		case Json::ValueType::UInt:
			value = _value.UInt;
			break;
		case Json::ValueType::Float:
			value = _value.Float;
			break;
		case Json::ValueType::String:
		case Json::ValueType::Object:
		case Json::ValueType::Array:
		default:
			return false;
		}
		return true;
	}

	bool JsonObject::AsInt(int& value) const
	{
		switch (GetType())
		{
		case Json::ValueType::Null:
			value = 0;
			break;
		case Json::ValueType::Bool:
			value = _value.Bool;
			break;
		case Json::ValueType::Int:
			value = _value.Int;
			break;
		case Json::ValueType::UInt:
			if (!IsInt())
				return false;
			value = _value.UInt;
			break;
		case Json::ValueType::Float:
			if (!IsInt())
				return false;
			value = static_cast<int>(_value.Float);
			break;
		case Json::ValueType::String:
		case Json::ValueType::Object:
		case Json::ValueType::Array:
		default:
			return false;
		}
		return true;
	}

	bool JsonObject::AsUInt(unsigned int& value) const
	{
		switch (GetType())
		{
		case Json::ValueType::Null:
			value = 0;
			break;
		case Json::ValueType::Bool:
			value = _value.Bool;
			break;
		case Json::ValueType::Int:
			if (!IsUInt())
				return false;
			value = _value.Int;
			break;
		case Json::ValueType::UInt:
			value = _value.UInt;
			break;
		case Json::ValueType::Float:
			if (!IsUInt())
				return false;
			value = static_cast<unsigned>(_value.Float);
			break;
		case Json::ValueType::String:
		case Json::ValueType::Object:
		case Json::ValueType::Array:
		default:
			return false;
		}
		return true;
	}

	bool JsonObject::AsFloat(float& value) const
	{
		switch (GetType())
		{
		case Json::ValueType::Null:
			value = 0.0f;
			break;
		case Json::ValueType::Bool:
			value = _value.Bool;
			break;
		case Json::ValueType::Int:
			if (!IsFloat())
				return false;
			value = static_cast<float>(_value.Int);
			break;
		case Json::ValueType::UInt:
			if (!IsFloat())
				return false;
			value = static_cast<float>(_value.UInt);
			break;
		case Json::ValueType::Float:
			value = _value.Float;
			break;
		case Json::ValueType::String:
		case Json::ValueType::Object:
		case Json::ValueType::Array:
		default:
			return false;
		}
		return true;
	}

	void JsonObject::CleanUp()
	{
		if (type == ValueType::Object && _value.Map != nullptr)
			delete _value.Map;
		else if (type == ValueType::Array && _value.Array != nullptr)
			delete _value.Array;
		else if (type == ValueType::String && _value.String != nullptr)
			ReleaseStringValue(_value.String);
	}

	char* JsonObject::DublicateStringValue(const char* value, unsigned int length)
	{
		// Whether 'value' is null terminated or not.
		bool isNullTerminated = (value[length - 1] == '\0') ? true : false;
		char* newString = static_cast<char*>(malloc(length + (isNullTerminated ? 0U : 1U)));
		ASSERT_TRUE((newString != nullptr), "Failed to allocate string value buffer");

		// Copying memory of 'value' to 'newString'.
		memcpy(newString, value, length);

		// If not null terminated, then add '\0' to the end of it.
		if (!isNullTerminated)
			newString[length] = '\0';
		return newString;
	}

	void JsonObject::ReleaseStringValue(char* value)
	{
		free(value);
	}
}
