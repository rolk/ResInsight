#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'job_queue_manager.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 
"""
Module implementing a queue for managing external jobs.

"""
import ctypes
from types import StringType, IntType
import time
from ert.cwrap import BaseCClass, CWrapper

from ert.job_queue import JOB_QUEUE_LIB, Job, JobStatusType



class JobQueueManager(BaseCClass):

    def __init__(self, queue):
        c_ptr = JobQueueManager.cNamespace().alloc(queue)
        super(JobQueueManager, self).__init__(c_ptr)


    def startQueue(self , total_size , verbose = False):
        JobQueueManager.cNamespace().start_queue( self , total_size , verbose )

    def getNumRunning(self):
        return JobQueueManager.cNamespace().get_num_running( self )

    def getNumComplete(self):
        return JobQueueManager.cNamespace().get_num_complete( self )

    def isRunning(self):
        return JobQueueManager.cNamespace().is_running( self )

    def free(self):
        JobQueueManager.cNamespace().free(self)

    def jobComplete(self , job_index):
        return JobQueueManager.cNamespace().job_complete( self , job_index )
        
    
        

#################################################################

cwrapper = CWrapper(JOB_QUEUE_LIB)
cwrapper.registerObjectType("job_queue_manager", JobQueueManager)

JobQueueManager.cNamespace().alloc             = cwrapper.prototype("c_void_p job_queue_manager_alloc( job_queue) ")
JobQueueManager.cNamespace().free              = cwrapper.prototype("void job_queue_manager_free( job_queue_manager )")
JobQueueManager.cNamespace().start_queue       = cwrapper.prototype("void job_queue_manager_start_queue( job_queue_manager , int , bool)")
JobQueueManager.cNamespace().get_num_running   = cwrapper.prototype("int job_queue_manager_get_num_running( job_queue_manager )")
JobQueueManager.cNamespace().get_num_complete  = cwrapper.prototype("int job_queue_manager_get_num_complete( job_queue_manager )")
JobQueueManager.cNamespace().is_running        = cwrapper.prototype("bool job_queue_manager_is_running( job_queue_manager )")
JobQueueManager.cNamespace().job_complete      = cwrapper.prototype("bool job_queue_manager_job_complete( job_queue_manager , int)")
