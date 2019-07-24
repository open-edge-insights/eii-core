# List of core IEI services and respective dockerignores.
services+=(ia_gobase)
services+=(ia_pybase)
services+=(ia_gopybase)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)
dockerignores+=(docker_setup/dockerignores/.dockerignore.common)

# List of Configurable IEI services and respective dockerignores.

while read line ; do
	services+=($line)
done < <(python3 get_services.py ./config/$IEI_SERVICES name)
while read line ; do
	dockerignores+=($line)
done < <(python3 get_services.py ./config/$IEI_SERVICES dockerignore)
