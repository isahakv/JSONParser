#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H
#include <istream>
#include <ostream>
#include <string>
#include <map>

using std::istream;
using std::ostream;
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
		static const JsonObject& NullSingleton();

	public:
		/** Create a Default JsonObject of the given type. */
		JsonObject(ValueType type = ValueType::Null);
		/** 'value' mast be null terminated. */
		JsonObject(const char* value);
		JsonObject(const string& value);
		JsonObject(bool value);
		JsonObject(int value);
		JsonObject(unsigned int value);
		JsonObject(float value);
		JsonObject(const JsonObject& other) = delete;
		JsonObject(JsonObject&& other) noexcept;
		~JsonObject();

		//Operator overloads.
		JsonObject& operator=(const JsonObject& other) = delete;
		JsonObject& operator=(JsonObject&& other) noexcept;

		/// Unary Operators.
		/** Access an object value by name
		* 'key' must be null-terminated. */
		const JsonObject& operator[](const char* key) const;
		const JsonObject& operator[](const string& key) const;
		/** Access an object value by name
		* 'key' must be null-terminated. */
		JsonObject& operator[](const char* key);
		JsonObject& operator[](const string& key);

		/// Binary Operators.
		friend ostream& operator<<(ostream& stream, const JsonObject& obj);
		friend istream& operator>>(istream& stream, const JsonObject& obj);

		void Append(const JsonObject& object);

		/** Getter for type. */
		inline ValueType GetType() const { return type; }

		inline bool IsString() const { return GetType() == ValueType::String; }

		bool AsString(string& value) const;

	private:
		ValueType type;

		struct KeyComparator
		{
			bool operator()(const string& key1, const string& key2) const
			{
				return key1.compare(key2);
			}
		};

		union ValueHolder
		{
			char* String;
			bool Bool;
			int Int;
			unsigned int UInt;
			float Float;
			map<string, JsonObject>* Map;
		} _value;

	private:
		void CleanUp();

		// String Helpers.
		/** 'value' must be null-terminated. */
		char* DublicateStringValue(const char* value, unsigned int length);
		/** Free the string duplicated by DublicateStringValue() */
		void ReleaseStringValue(char* value, unsigned int length = 0);
	};
}

#endif // !JSON_OBJECT_H
