from projectq.ops import All, CNOT, H, Measure, Rz, X, Z
from projectq import MainEngine
from projectq.meta import Dagger, Control
from projectq.backends import CircuitDrawer
from projectq.backends import ResourceCounter

def create_bell_pair(eng):
	b1 = eng.allocate_qubit()
	b2 = eng.allocate_qubit()

	H | b1
	CNOT | (b1, b2)

	return b1, b2


def run_teleport(eng, state_creation_function, verbose=False):
	b1, b2 = create_bell_pair(eng)

	psi = eng.allocate_qubit()
	if verbose:
		print("Alice is creating her state from scratch, i.e., |0>.")
	state_creation_function(eng, psi)

	CNOT | (psi, b1)
	if verbose:
		print("Alice entangled her qubit with her share of the Bell-pair.")

	H | psi
	Measure | psi
	Measure | b1

	msg_to_bob = [int(psi), int(b1)]
	if verbose:
		print("Alice is sending the message {} to Bob.".format(msg_to_bob))

	with Control(eng, b1):
		X | b2

	with Control(eng, psi):
		Z | b2

	if verbose:
		print("Bob is trying to uncompute the state.")
	with Dagger(eng):
		state_creation_function(eng,b2)

	del b2
	eng.flush()

	if verbose:
		print("Bob successfully arrived at |0>")

if __name__ == "__main__":
	drawing_engine = CircuitDrawer()
	resource_counter = ResourceCounter()
	#eng = MainEngine(drawing_engine) #tex drawing
	eng = MainEngine(backend=resource_counter) #resource counter
	
	def create_state(eng,qb):
		H | qb
		Rz(1.21) | qb

	run_teleport(eng, create_state, verbose=True)
	eng.flush()
	#print(drawing_engine.get_latex()) #tex drawing
	print(resource_counter) #resource counter