# List of core IEI services and respective dockerignores.
services+=(ia_log_rotate)
services+=(ia_gobase)
services+=(ia_pybase)
services+=(ia_data_agent)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(DataAgent/.dockerignore)

# List of Configurable IEI services and respective dockerignores.

while read line ; do
	services+=($line)
done < <(python3.6 get_services.py ./config/$IEI_SERVICES name)
while read line ; do
	dockerignores+=($line)
done < <(python3.6 get_services.py ./config/$IEI_SERVICES dockerignore)
