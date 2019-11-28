#include "JsonWriter.h"
#include <sstream>

namespace Json
{
	bool JsonWriter::Write(string& json, const JsonObject& root, bool styled)
	{
		if (root.GetType() != ValueType::Object && root.GetType() != ValueType::Array)
		{
			SetError("A valid JsonObject must be either an array or an object value.");
			return false;
		}

		this->styled = styled;
		return WriteValue(json, root);
	}

	bool JsonWriter::Write(ostream& os, const JsonObject& root, bool styled)
	{
		string json;
		if (!Write(json, root, styled))
			return false;

		os << json;
		return true;
	}

	bool JsonWriter::WriteValue(string& json, const JsonObject& object, int deepLevel)
	{
		bool succeed = true;
		switch (object.GetType())
		{
		case ValueType::Null:
		case ValueType::Bool:
		case ValueType::Int:
		case ValueType::UInt:
		case ValueType::Float:
			json += object.AsString();
			break;
		case ValueType::String:
			// If String, then add quotes.
			json += ToQuoticString(object.AsString());
			break;
		case ValueType::Object:
			succeed = styled ? WriteStyledObject(json, object, deepLevel) : WriteObject(json, object);
			break;
		case ValueType::Array:
			succeed = styled ? WriteStyledArray(json, object, deepLevel) : WriteArray(json, object);
			break;
		default:
			succeed = false;
			break;
		}
		return succeed;
	}

	bool JsonWriter::WriteObject(string& json, const JsonObject& object)
	{
		bool succeed = true;
		vector<string> names = object.GetMemberNames();
		int size = names.size();

		json += '{';
		for (int i = 0; i < size; i++)
		{
			string& name = names[i];
			json += ToQuoticString(name);
			json += ':';
			if (!(succeed = WriteValue(json, object[name])))
				break;
			json += (i == size - 1) ? '}' : ',';
		}
		return succeed;
	}

	bool JsonWriter::WriteArray(string& json, const JsonObject& object)
	{
		bool succeed = true;
		int size = object.Size();

		json += '[';
		for (int i = 0; i < size; i++)
		{
			if (!(succeed = WriteValue(json, object[i])))
				break;
			json += (i == size - 1) ? ']' : ',';
		}

		return succeed;
	}

	bool JsonWriter::WriteStyledObject(string& json, const JsonObject& object, int deepLevel)
	{
		bool succeed = true, isMultiLine = IsMultiLine(object);
		vector<string> names = object.GetMemberNames();
		int size = names.size();

		if (deepLevel > 0 && isMultiLine)
			json += '\n' + GetIndent(deepLevel);
		json += isMultiLine ? "{\n" + GetIndent(deepLevel + 1) : "{ ";
		for (int i = 0; i < size; i++)
		{
			string& name = names[i];
			json += ToQuoticString(name);
			json += " : ";
			if (!(succeed = WriteValue(json, object[name], deepLevel + 1)))
				break;

			// If last child.
			if (i == size - 1)
				json += isMultiLine ? "\n" + GetIndent(deepLevel) + "}" : " }";
			else
				json += isMultiLine ? ",\n" + GetIndent(deepLevel + 1) : ", ";
		}
		return succeed;
	}

	bool JsonWriter::WriteStyledArray(string& json, const JsonObject& object, int deepLevel)
	{
		bool succeed = true, isMultiLine = IsMultiLine(object);
		int size = object.Size();

		if (deepLevel > 0 && isMultiLine)
			json += '\n' + GetIndent(deepLevel);
		json += isMultiLine ? "[\n" + GetIndent(deepLevel + 1) : "[ ";
		for (int i = 0; i < size; i++)
		{
			if (!(succeed = WriteValue(json, object[i], deepLevel + 1)))
				break;

			// If last child.
			if (i == size - 1)
				json += isMultiLine ? "\n" + GetIndent(deepLevel) + "]" : " ]";
			else
				json += isMultiLine ? ",\n" + GetIndent(deepLevel + 1) : ", ";
		}

		return succeed;
	}

	string JsonWriter::ToQuoticString(const string& str)
	{
		string newStr = '"' + str + '"';
		return newStr;
	}

	string&& JsonWriter::ToQuoticString(string&& str)
	{
		str = '"' + str + '"';
		return std::move(str);
	}

	string JsonWriter::GetIndent(int deepLevel)
	{
		string tabs;
		tabs.reserve(deepLevel);
		for (int i = 0; i < deepLevel; i++)
			tabs += '\t';

		return tabs;
	}

	bool JsonWriter::IsMultiLine(const JsonObject& value)
	{
		if (value.GetType() == ValueType::Object)
		{
			JsonObject::ObjectConstIterator begin = value.ObjectConstBegin();
			JsonObject::ObjectConstIterator end = value.ObjectConstEnd();
			for (auto& itr = begin; itr != end; ++itr)
			{
				ValueType type = itr->second.GetType();
				// If 'value' has child with type object/array, return true.
				if (type == ValueType::Object || type == ValueType::Array)
					return true;
			}
		}
		else if (value.GetType() == ValueType::Array)
		{
			JsonObject::ArrayConstIterator begin = value.ArrayConstBegin();
			JsonObject::ArrayConstIterator end = value.ArrayConstEnd();
			for (auto& itr = begin; itr != end; ++itr)
			{
				ValueType type = itr->GetType();
				// If 'value' has child with type object/array, return true.
				if (type == ValueType::Object || type == ValueType::Array)
					return true;
			}
		}

		return false;
	}

	void JsonWriter::SetError(const char* message)
	{
		errorMessage = message;
	}
}