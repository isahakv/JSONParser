#ifndef JSON_READER_H
#define JSON_READER_H
#include <string>
#include <istream>
#include "JsonObject.h"

using std::string;
using std::istream;

namespace Json
{
	/** Class for Reading/Parsing JSON data from file/string to 
	*	'JsonObject' object. */
	class JsonReader
	{
	public:
		/** Parse JSON text from 'std::string' class. */
		bool Parse(const string& json, JsonObject& root);
		/** Parse JSON text from input stream. */
		bool Parse(istream& is, JsonObject& root);
		/** Parse JSON text from given begin to end. */
		bool Parse(const char* beginText, const char* endText, JsonObject& root);
		
		/** Get Error message. */
		inline const string& GetError() const { return errorMessage; }

	private:
		enum TokenType
		{
			tokenEndOfStream,
			tokenObjectBegin,
			tokenObjectEnd,
			tokenArrayBegin,
			tokenArrayEnd,
			tokenNull,
			tokenString,
			tokenTrue,
			tokenFalse,
			tokenNumber,
			tokenComma,
			tokenColon,
			tokenComment, // Not Supported Currently.
			tokenError
		};

		struct Token
		{
			TokenType type;
			const char* start;
			const char* end;
		};

	private:
		bool ReadValue(JsonObject& object);
		bool ReadToken(Token& token);
		bool ReadObject();
		bool ReadArray();

		/** Set Error message. */
		void SetError(const char* message);

		void SkipSpaces();

		// Deprecated.
		void RemoveWhiteSpaces(string& str);

	private:
		const char* begin;
		const char* current;
		const char* end;

		string errorMessage;
	};
}

#endif // !JSON_READER_H
