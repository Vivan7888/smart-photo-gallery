# Team work map

| Member | Main code folder | Primary output |
| --- | --- | --- |
| Member 1 | `data-structures/` | Linked list, queue, stack, search, and sort in C |
| Member 2 | `backend/` | C++ classes and local metadata handling |
| Member 3 | `frontend/` | Qt graphical interface |
| Member 4 | `photo-storage/` | Old-photo detection, archive/extract workflow, and statistics |

## Shared photo fields

Every module must use these fields for a photo: `id`, `fileName`, `filePath`, `dateAdded`, `fileSize`, `isArchived`, `archivePath`, and `albumName`.

## Rule for today

Work only in your own main code folder. Commit small working changes and push them to your feature branch. The final integration happens only after each module can run independently.
