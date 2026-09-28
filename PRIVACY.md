# RAD Privacy Policy

**Last updated: September 28, 2026**

## Overview

RAD is a local command-line tool for validating YAML files and performing a Kubernetes client-side dry run.

RAD is designed to process files locally on the user's device, It does not operate a cloud service or require a RAD server to perform its functionality.

## Information RAD Accesses

When a user provides a YAML file to RAD, RAD reads the selected file from the local filesystem in order to validate its YAML structure.

Depending on the command being used, RAD may access:

* The contents of the YAML file provided by the user.
* The local file path provided to RAD.
* The output produced by locally installed `kubectl`.

YAML files may contain configuration information, including information that could be sensitive. RAD does not intentionally collect or transmit this information to the RAD project maintainers.

## Local Processing

RAD processes YAML files locally on the user's device.

RAD does not upload YAML files, their contents, or validation results to any server or other remote service.

RAD does not require an internet connection to perform its YAML validation functionality.

After validation is complete, RAD does not retain a copy of the YAML file or create another file containing the YAML contents.

## Kubernetes Integration (`kubectl`)

RAD can optionally use the user's locally installed Kubernetes command-line tool, `kubectl`.

For this functionality, `kubectl` must already be installed and available in the user's local environment.

RAD performs kubernetes dry-run functionality for ConfigMaps, when `-d` flag pass in RAD, RAD invokes the locally installed `kubectl` command using the following operation:

```text
kubectl create cm policy --from-file=<file> --dry-run=client -o yaml
```

RAD provides the local file path to `kubectl`; RAD does not create a separate copy of the YAML file for this operation.

The `--dry-run=client` option performs the operation on the client side and does not create the ConfigMap in a Kubernetes cluster.

RAD captures the output produced by the locally installed `kubectl` process and displays that output to the user.

The behavior of `kubectl` itself, including any processing it performs based on the user's local Kubernetes configuration or environment, is outside of RAD's control and is governed by the user's installed version and configuration of `kubectl`.

Users should review their Kubernetes tooling and configuration separately when working with files containing sensitive information.

## Network Communication

RAD itself does not make network requests or communicate with RAD-operated servers or any remote servers.

The YAML validation functionality is performed locally.

RAD's invocation of `kubectl` is also performed locally. RAD does not itself establish a connection to a Kubernetes cluster.

Any network communication that may occur as a result of using locally installed external tools such as `kubectl` is controlled by those tools and their configuration, rather than by RAD.

## Storage and Retention

RAD does not intentionally store or retain copies of YAML files provided by the user.

RAD does not generate temporary files as part of its YAML validation or `kubectl` dry-run functionality.

RAD does not maintain a database or other remote storage containing user-provided YAML files or validation results.

Once RAD finishes processing the provided file, RAD does not retain the file contents.

## Logging

RAD does not maintain application logs containing the contents of user-provided YAML files.

RAD does not intentionally log passwords, credentials, secrets, or other YAML file contents.

The operating system, terminal, shell, or other locally installed tools may have their own logging or history mechanisms outside of RAD's control.

## Security

RAD is designed to perform its file processing locally and does not transmit user-provided YAML files to RAD-operated services or any remote servers.

However, users should treat YAML configuration files according to their own organization's security requirements. YAML files may contain credentials, tokens, endpoints, infrastructure configuration, or other sensitive information.

Users are responsible for ensuring that files provided to RAD and their local Kubernetes tooling are handled in accordance with their organization's security policies.

## Changes to This Privacy Policy

This privacy policy may be updated when RAD's functionality or data-handling practices change.

The latest version of this policy will be published in the RAD repository.

## Contact

For questions about this privacy policy or RAD's data-handling practices, please open an issue in the RAD GitHub repository.
