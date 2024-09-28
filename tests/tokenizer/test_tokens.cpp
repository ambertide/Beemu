#include <BeemuTokenTest.hpp>

namespace BeemuTests
{
	TEST_F(BeemuTokenTest, NOP)
	{
		auto json = nlohmann::json::parse("{\"type\":\"BEEMU_INSTRUCTION_TYPE_CPU_CONTROL\",\"duration_in_clock_cycles\":1,\"original_machine_code\":0,\"params\":{\"system_op\":\"BEEMU_CPU_OP_NOP\"}}");
		BeemuInstruction inst;
		::from_json(json, inst);
		ASSERT_EQ(inst.type, BEEMU_INSTRUCTION_TYPE_CPU_CONTROL);
		ASSERT_EQ(inst.params.system_op, BEEMU_CPU_OP_NOP);
	}
}