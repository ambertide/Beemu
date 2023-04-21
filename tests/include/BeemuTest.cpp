#include "BeemuTest.hpp"

void BeemuTests::BeemuTestFixture::SetUp()
{
	this->memory = beemu_memory_new(64000);
	this->registers = beemu_registers_new();
}

void BeemuTests::BeemuTestFixture::TearDown()
{
	beemu_memory_free(this->memory);
	beemu_registers_free(this->registers);
}