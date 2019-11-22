#ifndef JSON_WRITER_H
#define JSON_WRITER_H
#include <string>
#include <ostream>
#include "JsonObject.h"

namespace Json
{
	/** Class for Writing JSON data to file/string from 
	*	'JsonObject' object. */
	class JsonWriter
	{
	public:
		/** Write JsonObject to 'std::string' class. */
		bool Write(string& json, const JsonObject& root, bool styled = true);
		/** Write JsonObject to output stream. */
		bool Write(ostream& os, const JsonObject& root, bool styled = true);
		
		/** Get Error message. */
		inline const string& GetError() const { return errorMessage; }

	private:
		bool WriteValue(string& json, const JsonObject& object, int deepLevel = 0);
		bool WriteObject(string& json, const JsonObject& object);
		bool WriteArray(string& json, const JsonObject& object);
		bool WriteStyledObject(string& json, const JsonObject& object, int deepLevel);
		bool WriteStyledArray(string& json, const JsonObject& object, int deepLevel);

		/** Add quots to string. */
		string ToQuoticString(const string& str);
		string&& ToQuoticString(string&& str);

		/** Returns string of tabs depending on deep level. */
		string GetTabs(int deepLevel);

		/** 'value' must be of type object/array.
		* Returns true, if 'value' has child of type object/array,
		* Otherwise false. */
		bool IsMultiLine(const JsonObject& value);

		/** Set Error message. */
		void SetError(const char* message);

	private:
		bool styled = false;

		string errorMessage;
	};
}

#endif // !JSON_WRITER_H
