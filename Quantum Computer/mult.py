from projectq import MainEngine
from projectq.ops import H, CNOT, Swap
from projectq.backends import CircuitDrawer

#from teleport import create_bell_pair

def mul_con(eng):
	g0 = eng.allocate_qubit()
	g1 = eng.allocate_qubit()
	g2 = eng.allocate_qubit()
	g3 = eng.allocate_qubit()

	CNOT | (g2, g0)
	CNOT | (g2, g1)
	CNOT | (g3, g1)

	CNOT | (g0, g3)
	CNOT | (g1, g2)

	Swap | (g2, g3)

	return g0, g1, g2, g3


drawing_engine = CircuitDrawer()
eng = MainEngine(drawing_engine)

mul_con(eng)

eng.flush()
print(drawing_engine.get_latex())
