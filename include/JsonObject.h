#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H
#include <string>
#include <map>

#include <iostream>

using std::cout;
using std::string;
using std::map;

namespace Json
{
	enum class ValueType
	{
		Null,
		String,
		Boolean,
		Int,
		UInt,
		Float,
		Object,
		Array
	};

	class JsonObject
	{
	public:
		// static
		static const JsonObject& nullSingleton();

	public:
		/** Create a Default JsonObject of the given type. */
		JsonObject(ValueType type = ValueType::Null);
		JsonObject(const char* value);
		JsonObject(const string& value);
		JsonObject(bool value);
		JsonObject(int value);
		JsonObject(unsigned int value);
		JsonObject(float value);
		JsonObject(const JsonObject& value);
		JsonObject(JsonObject&& value);

		void Append(const JsonObject& object);

		/** Getter for type. */
		inline ValueType GetType() const { return type; }

		//Operator overloads.
		/** Access an object value by name */
		JsonObject& operator[](const char* key);
		const JsonObject& operator[](const char* key) const;

		JsonObject& operator[](const string& key);
		const JsonObject& operator[](const string& key) const;

	private:
		ValueType type;

		union ValueHolder
		{
			char* String;
			bool Bool;
			int Int;
			unsigned int UInt;
			float Float;
			map<char*, JsonObject>* Object;
		} value;
	};
}

#endif // !JSON_OBJECT_H
