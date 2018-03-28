#include <Python.h>
#include <sched.h>

static PyObject *
container_unshare(PyObject *self, PyObject *args)
{
	int flags;
	int res;

	if (!PyArg_ParseTuple(args, "i", &flags))
	return NULL;
	res = unshare(flags);
	if (res < 0) {
		return PyErr_SetFromErrno(PyExc_OSError);
	}
	return Py_BuildValue("i", res);
}

static PyMethodDef ContainerMethods[] = {
	{"unshare",  container_unshare, METH_VARARGS,
		"Run unshare call in current process"},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initcontainer(void)
{
	PyObject *m;

	m = Py_InitModule("container", ContainerMethods);
	if (m == NULL)
		return;
}
