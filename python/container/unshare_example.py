import container
import subprocess

container.unshare(container.CLONE_NEWNET)

result = subprocess.run(['ip', 'addr'], capture_output=True, text=True)

# Check if the command was successful
if result.returncode == 0:
    print("Command executed successfully!")
    print("Output:\n", result.stdout)
else:
    print("Error:", result.stderr)
