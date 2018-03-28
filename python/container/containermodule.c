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

	PyModule_AddIntConstant(m, "CLONE_FILES", CLONE_FILES);
	PyModule_AddIntConstant(m, "CLONE_FS", CLONE_FS);
	PyModule_AddIntConstant(m, "CLONE_NEWCGROUP", CLONE_NEWCGROUP);
	PyModule_AddIntConstant(m, "CLONE_NEWIPC", CLONE_NEWIPC);
	PyModule_AddIntConstant(m, "CLONE_NEWNET", CLONE_NEWNET);
	PyModule_AddIntConstant(m, "CLONE_NEWNS", CLONE_NEWNS);
	PyModule_AddIntConstant(m, "CLONE_NEWPID", CLONE_NEWPID);
	PyModule_AddIntConstant(m, "CLONE_NEWUSER", CLONE_NEWUSER);
	PyModule_AddIntConstant(m, "CLONE_NEWUTS", CLONE_NEWUTS);
	PyModule_AddIntConstant(m, "CLONE_SYSVSEM", CLONE_SYSVSEM);
}
