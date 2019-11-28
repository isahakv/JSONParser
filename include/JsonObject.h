#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <unordered_map>

using std::istream;
using std::ostream;
using std::string;
using std::vector;
using std::unordered_map;

class JsonObject;

namespace Json
{
	// All possible Types of Json Object.
	enum class ValueType : uint8_t
	{
		Null,
		String,
		Bool,
		Int,
		UInt,
		Float,
		Object,
		Array
	};

	class JsonObject
	{
	public:
		// static object that is considered as null.
		static const JsonObject& NullSingleton();

		/** Create a Default JsonObject of the given type. */
		constexpr JsonObject(ValueType type = ValueType::Null);
		/** 'value' mast be null terminated. */
		JsonObject(const char* value);
		JsonObject(const string& value);
		constexpr JsonObject(bool value)
			: type(ValueType::Bool), _value(value) {}
		constexpr JsonObject(int value)
			: type(ValueType::Int), _value(value) {}
		constexpr JsonObject(unsigned int value)
			: type(ValueType::UInt), _value(value) {}
		constexpr JsonObject(float value)
			: type(ValueType::Float), _value(value) {}
		JsonObject(const JsonObject& other);
		constexpr JsonObject(JsonObject&& other) noexcept;
		~JsonObject();

#pragma region UnaryOperators
		/** Access an object value by name.
		* 'key' must be null-terminated. */
		const JsonObject& operator[](const char* key) const;
		const JsonObject& operator[](const string& key) const;

		/** Access an object value by name.
		* 'key' must be null-terminated. */
		JsonObject& operator[](const char* key);
		JsonObject& operator[](const string& key);

		/** Access an array element by index.
		* Returns 'NullSingleton' if out of range. */
		const JsonObject& operator[](int index) const;
		JsonObject& operator[](int index);
#pragma endregion

#pragma region BinaryOperator
		/** Copy assignment operator. */
		JsonObject& operator=(const JsonObject& other) = delete;
		/** Move assignment operator. */
		JsonObject& operator=(JsonObject&& other) noexcept;

		/** Equal operator. */
		bool operator==(const JsonObject& other) const;
		/** Not Equal operator. */
		bool operator!=(const JsonObject& other) const;
#pragma endregion

#pragma region Iteration
		using ObjectIterator = unordered_map<string, JsonObject>::iterator;
		using ObjectConstIterator = unordered_map<string, JsonObject>::const_iterator;
		using ArrayIterator = vector<JsonObject>::iterator;
		using ArrayConstIterator = vector<JsonObject>::const_iterator;

		ObjectIterator ObjectBegin();
		ObjectConstIterator ObjectConstBegin() const;
		ObjectIterator ObjectEnd();
		ObjectConstIterator ObjectConstEnd() const;

		ArrayIterator ArrayBegin();
		ArrayConstIterator ArrayConstBegin() const;
		ArrayIterator ArrayEnd();
		ArrayConstIterator ArrayConstEnd() const;
#pragma endregion

		/** Add value as a member to this array. */
		void Append(const JsonObject& object);
		/** Add value as a member to this array. */
		void Append(JsonObject&& object);

		/** Return number of values in object/array.
		* Return 0 if this object is not object/array type. */
		int Size() const;

		/** Return true if object/array has no members, otherwise false. */
		inline bool IsEmpty() const { return Size() == 0; }

		/** Clears all members from object/array. */
		void Clear();

		/** Resize array size. */
		void Resize(unsigned int newSize);

		/** Return true if given object/array index is valid, otherwise false. */
		bool IsValidIndex(int index) const;

		/** Insert value to this array at specified index.
		* Returns true if insertion completes successfully,
		* Returns false if this object is not array or
		* index is out of range */
		bool Insert(int index, const JsonObject& object);
		bool Insert(int index, JsonObject&& object);

		/** Remove member by specified key,
		* key may not be null-terminated. */
		bool RemoveMember(const char* key, JsonObject* removed);
		/** Remove member by specified key. */
		bool RemoveMember(const string& key, JsonObject* removed);
		
		/** Remove element at specified index. */
		bool RemoveIndex(int index, JsonObject* removed);

		/** Return true if this object has member of specified key,
		* otherwise false.
		*key may not be null - terminated. */
		bool IsMember(const char* key) const;
		/** Return true if this object has member of specified key,
		* otherwise false. */
		bool IsMember(const string& key) const;

		/** Returns names of all members of this object,
		* Returns empty vector if this is not object type or
		* there is no members. */
		vector<string> GetMemberNames() const;

		/** Getter for object type. */
		inline ValueType GetType() const { return type; }

#pragma region Conversion Checkers
		bool IsIntegral(float value) const;
		bool IsInt() const;
		bool IsUInt() const;
		bool IsFloat() const;
		bool IsNumeric() const;
		inline bool IsNull() const { return GetType() == ValueType::Null; }
		inline bool IsString() const { return GetType() == ValueType::String; }
		inline bool IsBool() const { return GetType() == ValueType::Bool; }
		inline bool IsObject() const { return GetType() == ValueType::Object; }
		inline bool IsArray() const { return GetType() == ValueType::Array; }
		bool IsConvertibleTo(ValueType otherType) const;
#pragma endregion
		
#pragma region Converters
		string AsString() const;
		bool AsBool() const;
		int AsInt() const;
		unsigned int AsUInt() const;
		float AsFloat() const;

		bool AsString(string& value) const;
		bool AsBool(bool& value) const;
		bool AsInt(int& value) const;
		bool AsUInt(unsigned int& value) const;
		bool AsFloat(float& value) const;
#pragma endregion

	private:
		ValueType type;

		union ValueHolder
		{
			constexpr ValueHolder() : Map(nullptr) {}
			constexpr ValueHolder(bool Bool) : Bool(Bool) {}
			constexpr ValueHolder(int Int) : Int(Int) {}
			constexpr ValueHolder(unsigned int UInt) : UInt(UInt) {}
			constexpr ValueHolder(float Float) : Float(Float) {}

			char* String;
			bool Bool;
			int Int;
			unsigned int UInt;
			float Float;
			unordered_map<string, JsonObject>* Map;
			vector<JsonObject>* Array;
		} _value;

	private:
		/** Cleans up memory. */
		void CleanUp();

		// String Helpers.
		/** 'value' must be null-terminated. */
		char* DublicateStringValue(const char* value, unsigned int length);
		/** Free the string duplicated by DublicateStringValue() */
		void ReleaseStringValue(char* value);
	};
}

#endif // !JSON_OBJECT_H
