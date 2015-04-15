#include <include/operators/meshoperator.h>
#include <include/resources/resourcemanager.h>

MeshOperator::MeshOperator() {}

MeshOperator::~MeshOperator()
{
	if (Value) TheResourceManager->DiscardMesh(Value);
}

void MeshOperator::SetValue( Mesh* const & NewValue )
{
	if (Value) TheResourceManager->DiscardMesh(Value);
	ValueOperator::SetValue(NewValue);
}

MeshOperator* MeshOperator::Create( OWNERSHIP Mesh* _Value )
{
	MeshOperator* op = new MeshOperator();
	op->SetValue(_Value);
	return op;
}
