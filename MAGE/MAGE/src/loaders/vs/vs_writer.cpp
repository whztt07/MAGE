//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "loaders\vs\vs_writer.hpp"
#include "loaders\vs\vs_tokens.hpp"
#include "utils\logging\error.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Definitions
//-----------------------------------------------------------------------------
namespace mage {

	VSWriter::VSWriter(const vector< Variable > &variable_buffer)
		: Writer(), 
		m_variable_buffer(variable_buffer) {}

	VSWriter::VSWriter(VSWriter &&writer) = default;

	VSWriter::~VSWriter() = default;

	void VSWriter::Write() {
		
		char output[MAX_PATH];
		const size_t output_count = static_cast< size_t >(_countof(output));

		for (const auto &variable : m_variable_buffer) {
			
			const char *name      = variable.GetName().c_str();
			const void *raw_value = variable.GetValue();

			switch (variable.GetType()) {
			
			case VariableType::Bool: {
				
				const bool *value = static_cast< const bool * >(raw_value);
				if (*value) {
					sprintf_s(output, output_count, "%s %s true", 
						MAGE_VS_TOKEN_BOOL, name);
				}
				else {
					sprintf_s(output, output_count, "%s %s false", 
						MAGE_VS_TOKEN_BOOL, name);
				}
				
				break;
			}
			
			case VariableType::Int: {
				
				const S32 *value = static_cast< const S32 * >(raw_value);
				sprintf_s(output, output_count, "%s %s %d", 
					MAGE_VS_TOKEN_INT, name, *value);
				
				break;
			}
			
			case VariableType::Int2: {
				
				const XMINT2 *value = static_cast< const XMINT2 * >(raw_value);
				sprintf_s(output, output_count, "%s %s %d %d", 
					MAGE_VS_TOKEN_INT2, name, value->x, value->y);
				
				break;
			}
			
			case VariableType::Int3: {
				
				const XMINT3 *value = static_cast< const XMINT3 * >(raw_value);
				sprintf_s(output, output_count, "%s %s %d %d %d", 
					MAGE_VS_TOKEN_INT3, name, value->x, value->y, value->z);
				
				break;
			}
			
			case VariableType::Float: {
				
				const F32 *value = static_cast< const F32 * >(raw_value);
				sprintf_s(output, output_count, "%s %s %f", 
					MAGE_VS_TOKEN_FLOAT, name, *value);
				
				break;
			}
			
			case VariableType::Float2: {
				
				const XMFLOAT2 *value = static_cast< const XMFLOAT2 * >(raw_value);
				sprintf_s(output, output_count, "%s %s %f %f", 
					MAGE_VS_TOKEN_FLOAT2, name, value->x, value->y);
				
				break;
			}
			
			case VariableType::Float3: {
				
				const XMFLOAT3 *value = static_cast< const XMFLOAT3 * >(raw_value);
				sprintf_s(output, output_count, "%s %s %f %f %f", 
					MAGE_VS_TOKEN_FLOAT3, name, value->x, value->y, value->z);
				
				break;
			}
			
			case VariableType::Float4: {
				
				const XMFLOAT4 *value = static_cast< const XMFLOAT4 * >(raw_value);
				sprintf_s(output, output_count, "%s %s %f %f %f %f", 
					MAGE_VS_TOKEN_FLOAT4, name, value->x, value->y, value->z, value->w);
				
				break;
			}
			
			case VariableType::Color: {
				
				const XMFLOAT4 *value = static_cast< const XMFLOAT4 * >(raw_value);
				sprintf_s(output, output_count, "%s %s %f %f %f %f", 
					MAGE_VS_TOKEN_COLOR, name, value->x, value->y, value->z, value->w);
				
				break;
			}
			
			case VariableType::String: {
				
				const string *value = static_cast< const string * >(raw_value);
				sprintf_s(output, output_count, "%s %s \"%s\"", 
					MAGE_VS_TOKEN_STRING, name, value->c_str());
				
				break;
			}
			
			default: {
				
				Warning("%ls: could not export variable: %s", GetFilename().c_str(), name);
				
				return;
			}
			}

			WriteStringLine(output);
		}
	}
}