#include "JsonObject.h"
#include "Assertions.h"

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
		_value.Map = nullptr;
		switch (type)
		{
		case Json::ValueType::Null:
			_value.Map = nullptr;
			break;
		case Json::ValueType::String:
			_value.String = nullptr;
			break;
		case Json::ValueType::Boolean:
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
		case Json::ValueType::Array:
			_value.Map = new map<string, JsonObject>();
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
		type = ValueType::Boolean;
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
	
	JsonObject::JsonObject(JsonObject&& other) noexcept
		: type(other.type), _value(std::move(other._value))
	{
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

	const JsonObject& JsonObject::operator[](const char* key) const
	{
		string sKey = key;
		return (*this)[sKey];
	}

	const JsonObject& JsonObject::operator[](const string& key) const
	{
		ASSERT_TRUE((GetType() == ValueType::Object || GetType() == ValueType::Null),
			"in JsonObject::operator[] requires ObjectValue or NullValue!");

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

		// If is NullValue, then make ObjectValue.
		if (GetType() == ValueType::Null)
			(*this) = JsonObject(ValueType::Object);

		auto itr = _value.Map->lower_bound(key);
		if (itr != _value.Map->end()) // If exists.
			return itr->second;

		std::pair<string, JsonObject> defaultValue(key, JsonObject());
		itr = _value.Map->insert(itr, std::move(defaultValue));
		return itr->second;
	}

	void JsonObject::Append(const JsonObject& object)
	{

	}

	bool JsonObject::AsString(string& value) const
	{
		switch (GetType())
		{
		case Json::ValueType::Null:
			value = "";
		case Json::ValueType::String:
			value = _value.String;
			break;
		case Json::ValueType::Boolean:
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
			break;
		}
		return true;
	}
	
	void JsonObject::CleanUp()
	{
		if (type == ValueType::Object && _value.Map != nullptr)
			delete _value.Map;
		else if (type == ValueType::String && _value.String != nullptr)
			ReleaseStringValue(_value.String);
	}

	char* JsonObject::DublicateStringValue(const char* value, unsigned int length)
	{
		char* newString = static_cast<char*>(malloc(length + 1U));
		ASSERT_TRUE((newString != nullptr), "Failed to allocate string value buffer");
		
		memcpy(newString, value, length);
		newString[length] = '\0';
		return newString;
	}
	
	void JsonObject::ReleaseStringValue(char* value, unsigned int length)
	{
		length = (length == 0) ? strlen(value) : length;
		free(value);
	}
}
