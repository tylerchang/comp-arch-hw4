from google.cloud import compute_v1
import time

def create_instance(project_id, zone, instance_name, num_cores):
    instance_client = compute_v1.InstancesClient()
    
    # Configure network interface
    network_interface = compute_v1.NetworkInterface()
    network_interface.network = "global/networks/default"
    
    # Add external access
    access = compute_v1.AccessConfig()
    access.type_ = compute_v1.AccessConfig.Type.ONE_TO_ONE_NAT.name
    access.name = "External NAT"
    access.network_tier = compute_v1.AccessConfig.NetworkTier.PREMIUM.name
    network_interface.access_configs = [access]

    # Create boot disk
    disk = compute_v1.AttachedDisk()
    disk.boot = True
    disk.auto_delete = True
    initialize_params = compute_v1.AttachedDiskInitializeParams()
    initialize_params.source_image = "projects/debian-cloud/global/images/debian-11-bullseye-v20241112"
    disk.initialize_params = initialize_params
    
    # Create instance
    instance = compute_v1.Instance()
    instance.name = instance_name
    instance.network_interfaces = [network_interface]
    instance.disks = [disk]
    instance.machine_type = f"zones/{zone}/machineTypes/e2-standard-{num_cores}"
    
    # Add metadata (startup script)
    instance.metadata = compute_v1.Metadata()
    instance.metadata.items = [
        compute_v1.Items(
            key="startup-script",
            value=open("qs_startup-script.sh", "r").read()
        )
    ]

    # Add service account
    instance.service_accounts = [
        compute_v1.ServiceAccount(
            email="default",
            scopes=[
                "https://www.googleapis.com/auth/devstorage.read_write",
                "https://www.googleapis.com/auth/logging.write",
                "https://www.googleapis.com/auth/compute"
            ]
        )
    ]

    # Create request
    request = compute_v1.InsertInstanceRequest()
    request.zone = zone
    request.project = project_id
    request.instance_resource = instance

    # Create the instance
    operation = instance_client.insert(request=request)
    return operation

def main():
    project_id = "comp-arch-columbia"
    zone = "us-central1-a"
    
    # Available core counts for e2-standard machines
    available_cores = [2, 4, 8, 16]
    
    for cores in available_cores:
        instance_name = f"quicksort-instance-{cores}core"
        print(f"Creating instance {instance_name}...")
        operation = create_instance(
            project_id=project_id,
            zone=zone,
            instance_name=instance_name,
            num_cores=cores
        )
        # Wait for operation to complete
        operation.result()
        print(f"Instance {instance_name} created successfully")
        time.sleep(2)  # Add delay to avoid rate limiting

if __name__ == "__main__":
    main()